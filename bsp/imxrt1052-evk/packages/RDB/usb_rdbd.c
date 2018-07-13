/*
 * File      : usb_rdbd.c
 * COPYRIGHT (C) 2008 - 2016, RT-Thread Development Team
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-11-16     ZYH          first version
 */
#include <rtthread.h>
#define PKGS_USING_USB_RDBD
#ifdef PKGS_USING_USB_RDBD
#include <rthw.h>
#include <rtservice.h>
#include <rtdevice.h>
#include <drivers/usb_device.h>
#include "rdbd.h"
#include "usb_rdbd.h"

#define DBG_ENABLE
#define DBG_SECTION_NAME  "[RDBD]"
#define DBG_LEVEL         DBG_WARNING
#define DBG_COLOR
#include <rtdbg.h>

static uep_t rdbd_ep_out;
static uep_t rdbd_ep_in;
static struct rt_completion rdbd_ep_in_completion;
static rt_uint8_t cmd_buff[64];
static ufunction_t rdbd_func = RT_NULL;
static rdbd_device_t rdbd;

static struct udevice_descriptor dev_desc =
{
    USB_DESC_LENGTH_DEVICE,     //bLength;
    USB_DESC_TYPE_DEVICE,       //type;
    USB_BCD_VERSION,            //bcdUSB;
    0x00,                       //bDeviceClass;
    0x00,                       //bDeviceSubClass;
    0x00,                       //bDeviceProtocol;
    0x40,                       //bMaxPacketSize0;
    _VENDOR_ID,                 //idVendor;
    _PRODUCT_ID,                //idProduct;
    USB_BCD_DEVICE,             //bcdDevice;
    USB_STRING_MANU_INDEX,      //iManufacturer;
    USB_STRING_PRODUCT_INDEX,   //iProduct;
    USB_STRING_SERIAL_INDEX,    //iSerialNumber;
    USB_DYNAMIC,                //bNumConfigurations;
};
//FS and HS needed
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),          //bLength
    USB_DESC_TYPE_DEVICEQUALIFIER,  //bDescriptorType
    0x0200,                         //bcdUSB
    0xFF,                           //bDeviceClass
    0x00,                           //bDeviceSubClass
    0x00,                           //bDeviceProtocol
    64,                             //bMaxPacketSize0
    0x01,                           //bNumConfigurations
    0,
};

struct usb_rdbd_descriptor _usb_rdbd_desc =
{
#ifdef RT_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    USB_DESC_LENGTH_IAD,
    USB_DESC_TYPE_IAD,
    USB_DYNAMIC,
    0x01,
    0xFF,
    0x00,
    0x00,
    0x00,
#endif
    /*interface descriptor*/
    USB_DESC_LENGTH_INTERFACE,  //bLength;
    USB_DESC_TYPE_INTERFACE,    //type;
    USB_DYNAMIC,                //bInterfaceNumber;
    0x00,                       //bAlternateSetting;
    0x02,                       //bNumEndpoints
    0xFF,                       //bInterfaceClass;
    0x00,                       //bInterfaceSubClass;
    0x00,                       //bInterfaceProtocol;
    0x00,                       //iInterface;
    /*endpoint descriptor*/
    USB_DESC_LENGTH_ENDPOINT,
    USB_DESC_TYPE_ENDPOINT,
    USB_DYNAMIC | USB_DIR_OUT,
    USB_EP_ATTR_BULK,
    USB_DYNAMIC,
    0x00,
    /*endpoint descriptor*/
    USB_DESC_LENGTH_ENDPOINT,
    USB_DESC_TYPE_ENDPOINT,
    USB_DYNAMIC | USB_DIR_IN,
    USB_EP_ATTR_BULK,
    USB_DYNAMIC,
    0x00,
};


const static char *_ustring[] =
{
    "Language",
    "RT-Thread Team.",
    "RT-Thread Debug Bridge",
    "32021919830108",
    "Configuration",
    "Interface",
    USB_STRING_OS//must be
};
struct usb_os_proerty usb_rdbd_proerty[] =
{
    USB_OS_PROERTY_DESC(USB_OS_PROERTY_TYPE_REG_SZ, "DeviceInterfaceGUID", "{af2c84af-785c-4aba-ad24-72c5bbcd0504}"),
};

struct usb_os_function_comp_id_descriptor rdbd_usb_func_comp_id_desc =
{
    .bFirstInterfaceNumber = USB_DYNAMIC,
    .reserved1          = 0x01,
    .compatibleID       = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00},
    .subCompatibleID    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .reserved2          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static rt_err_t _ep_out_handler(ufunction_t func, rt_size_t size)
{
    rdbd->msg_receiver(rdbd, (const char *)rdbd_ep_out->buffer);
    rdbd_ep_out->request.buffer = rdbd_ep_out->buffer;
    rdbd_ep_out->request.size = EP_MAXPACKET(rdbd_ep_out);
    rdbd_ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    rt_usbd_io_request(func->device, rdbd_ep_out, &rdbd_ep_out->request);
    return RT_EOK;
}

static rt_err_t _ep_in_handler(ufunction_t func, rt_size_t size)
{
    rt_completion_done(&rdbd_ep_in_completion);
    return RT_EOK;
}

static rt_err_t _ep0_cmd_handler(udevice_t device, rt_size_t size)
{
    dcd_ep0_send_status(device->dcd);
    return RT_EOK;
}
static rt_err_t _ep0_cmd_read(ufunction_t func, ureq_t setup)
{

    return RT_EOK;
}
static rt_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    rt_uint8_t *service_list;
    rt_uint8_t pipe_addr;
    switch (setup->bRequest)
    {
    case 'A':
        switch (setup->wIndex)
        {
        case 0x05:
            usbd_os_proerty_descriptor_send(func, setup, usb_rdbd_proerty, sizeof(usb_rdbd_proerty) / sizeof(usb_rdbd_proerty[0]));
            break;
        }
        break;
    case RDBD_CMD_GET_SERVICE_LIST:
        service_list = rdbd->get_service_list(rdbd);
        if(service_list == RT_NULL)
        {
            LOG_E("No memory to create service list");
            rt_usbd_ep0_set_stall(func->device);
        }
        rt_usbd_ep0_write(func->device, service_list, setup->wLength);
        rt_free(service_list);
        service_list = RT_NULL;
        break;
    case RDBD_CMD_GET_SERVICE_PIPE_ADDR:
        pipe_addr = rdbd->get_pipe_addr(rdbd, setup->wValue >> 8, setup->wValue & 0x80ff);
        if(pipe_addr == 0xff)
        {
            LOG_E("not found pipe service: 0x%02X, dir :", setup->wValue >> 8, ((setup->wValue & 0x80ff) == 0x80) ? "device to host" : "host to device");
            rt_usbd_ep0_set_stall(func->device);
            return RT_ERROR;
        }
        rt_usbd_ep0_write(func->device, &pipe_addr, setup->wLength);
        break;
    default:
        LOG_E("unsupport request 0x%02X", setup->bRequest);
        rt_usbd_ep0_set_stall(func->device);
        break;
    }
    return RT_EOK;
}
static rt_err_t _function_enable(ufunction_t func)
{
    RT_ASSERT(func != RT_NULL);
    if (rdbd_ep_out->buffer == RT_NULL)
    {
        rdbd_ep_out->buffer = rt_malloc(EP_MAXPACKET(rdbd_ep_out));
        if(rdbd_ep_out->buffer == RT_NULL)
        {
            LOG_E("No memory to config usb");
            return RT_ERROR;
        }
    }
    rdbd_ep_out->request.buffer = rdbd_ep_out->buffer;
    rdbd_ep_out->request.size = EP_MAXPACKET(rdbd_ep_out);
    rdbd_ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    rt_usbd_io_request(func->device, rdbd_ep_out, &rdbd_ep_out->request);
    return RT_EOK;
}
static rt_err_t _function_disable(ufunction_t func)
{
    RT_ASSERT(func != RT_NULL);
    if (rdbd_ep_out->buffer != RT_NULL)
    {
        rt_free(rdbd_ep_out->buffer);
        rdbd_ep_out->buffer = RT_NULL;
    }
    return RT_EOK;
}

static struct ufunction_ops ops =
{
    _function_enable,
    _function_disable,
    RT_NULL,
};

static rt_err_t _rdbd_usb_descriptor_config(usb_rdbd_desc_t rdbd_desc, rt_uint8_t cintf_nr, rt_uint8_t device_is_hs)
{
#ifdef RT_USB_DEVICE_COMPOSITE
    rdbd_desc->iad_desc.bFirstInterface = cintf_nr;
#endif
    rdbd_desc->ep_out_desc.wMaxPacketSize = device_is_hs ? 512 : 64;
    rdbd_desc->ep_in_desc.wMaxPacketSize = device_is_hs ? 512 : 64;
    rdbd_usb_func_comp_id_desc.bFirstInterfaceNumber = cintf_nr;
    return RT_EOK;
}

static rt_size_t rdbd_usb_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    if (rdbd_func->device->state != USB_STATE_CONFIGURED)
    {
        return 0;
    }
    rdbd_ep_in->buffer = (void *)buffer;
    rdbd_ep_in->request.buffer = rdbd_ep_in->buffer;
    rdbd_ep_in->request.size = size;
    rdbd_ep_in->request.req_type = UIO_REQUEST_WRITE;
    rt_completion_init(&rdbd_ep_in_completion);
    rt_usbd_io_request(rdbd_func->device, rdbd_ep_in, &rdbd_ep_in->request);
    rt_completion_wait(&rdbd_ep_in_completion, RT_WAITING_FOREVER);
    return size;
}

static rt_err_t rt_usb_rdbd_usb_init(ufunction_t func)
{
    rdbd = rdbd_device_register("urdbd", rdbd_usb_write, 127);
    if (rdbd == RT_NULL)
    {
        LOG_E("Create rdbd device error");
        return RT_ERROR;
    }
}

ufunction_t rt_usbd_function_rdbd_usb_create(udevice_t device)
{
    uintf_t             rdbd_usb_intf;
    ualtsetting_t       rdbd_usb_setting;
    usb_rdbd_desc_t       rdbd_usb_desc;

    /* parameter check */
    RT_ASSERT(device != RT_NULL);

    /* set usb device string description */
    rt_usbd_device_set_string(device, _ustring);

    /* create a cdc function */
    rdbd_func = rt_usbd_function_new(device, &dev_desc, &ops);
    rt_usbd_device_set_qualifier(device, &dev_qualifier);

    /* create an interface object */
    rdbd_usb_intf = rt_usbd_interface_new(device, _interface_handler);

    /* create an alternate setting object */
    rdbd_usb_setting = rt_usbd_altsetting_new(sizeof(struct usb_rdbd_descriptor));

    /* config desc in alternate setting */
    rt_usbd_altsetting_config_descriptor(rdbd_usb_setting, &_usb_rdbd_desc, (rt_off_t) & ((usb_rdbd_desc_t)0)->intf_desc);

    /* configure the hid interface descriptor */
    _rdbd_usb_descriptor_config(rdbd_usb_setting->desc, rdbd_usb_intf->intf_num, device->dcd->device_is_hs);

    /* create endpoint */
    rdbd_usb_desc = (usb_rdbd_desc_t)rdbd_usb_setting->desc;
    rdbd_ep_out = rt_usbd_endpoint_new(&rdbd_usb_desc->ep_out_desc, _ep_out_handler);
    rdbd_ep_in  = rt_usbd_endpoint_new(&rdbd_usb_desc->ep_in_desc, _ep_in_handler);

    /* add the int out and int in endpoint to the alternate setting */
    rt_usbd_altsetting_add_endpoint(rdbd_usb_setting, rdbd_ep_out);
    rt_usbd_altsetting_add_endpoint(rdbd_usb_setting, rdbd_ep_in);

    /* add the alternate setting to the interface, then set default setting */
    rt_usbd_interface_add_altsetting(rdbd_usb_intf, rdbd_usb_setting);
    rt_usbd_set_altsetting(rdbd_usb_intf, 0);

    /* add the interface to the mass storage function */
    rt_usbd_function_add_interface(rdbd_func, rdbd_usb_intf);

    rt_usbd_os_comp_id_desc_add_os_func_comp_id_desc(device->os_comp_id_desc, &rdbd_usb_func_comp_id_desc);
    /* initilize winusb */
    rt_usb_rdbd_usb_init(rdbd_func);
    return rdbd_func;
}

struct udclass rdbd_usb_class =
{
    .rt_usbd_function_create = rt_usbd_function_rdbd_usb_create
};

int rt_usbd_rdbd_usb_class_register(void)
{
    rt_usbd_class_register(&rdbd_usb_class);
    return 0;
}
INIT_PREV_EXPORT(rt_usbd_rdbd_usb_class_register);

#endif
