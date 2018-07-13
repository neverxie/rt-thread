#ifndef __USB_RDBD_H__
#define __USB_RDBD_H__
#include <rdbd.h>
struct usb_rdbd_descriptor
{
#ifdef RT_USB_DEVICE_COMPOSITE
    struct uiad_descriptor iad_desc;
#endif
    struct uinterface_descriptor intf_desc;
    struct uendpoint_descriptor ep_out_desc;
    struct uendpoint_descriptor ep_in_desc;
};
typedef struct usb_rdbd_descriptor* usb_rdbd_desc_t;

#define RDBD_CMD_GET_SERVICE_LIST      0x80
#define RDBD_CMD_GET_SERVICE_PIPE_ADDR 0x81
#endif
