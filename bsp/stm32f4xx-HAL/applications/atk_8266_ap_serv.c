/*
 * File
 *
 * Change Logs:
 * Date         Author      Notes
 *
 */

#include <rtthread.h>
#include <drv_led.h>
#include <at.h>
static void atk_8266_ap_serv_entry(void *param) {
    uint8_t chan = 1;
    uint8_t wpa = 4;

    at_exec_cmd(param, "AT+CWMODE=2");
    at_exec_cmd(param, "AT+RST");
    at_exec_cmd(param, "AT+CWSAP=\"never\",\"wangjimima00\",1,4");
    at_exec_cmd(param, "AT+CIPMUX=1");
    at_exec_cmd(param, "AT+CIPSERVER=1,8088");

    while (1) {
        at_exec_cmd(param, "AT");
        if (at_resp_get_line_by_kw(param, "OK") != NULL)
            LD2_ON;
        rt_thread_delay(500);
    }
}

static int atk_8266_ap_serv_init(void) {
    rt_thread_t tid;
    at_response_t resp;

    led_init();
    at_client_init("uart1", 512);

    resp = at_create_resp(512, 0, rt_tick_from_millisecond(5000));
    if (resp == NULL) {
        rt_kprintf("at resp create fail\n");
        goto __exit;
    }

    tid = rt_thread_create("apse", atk_8266_ap_serv_entry, (at_response_t)resp,\
            1024, 10, 10);
    if (tid == NULL) {
        rt_kprintf("atk_8266 sta serv thread create failed!\n");
        goto __exit;
    }

    rt_thread_startup(tid);

    return 0;

__exit:
    at_delete_resp(resp);
    return -1;
}
INIT_APP_EXPORT(atk_8266_ap_serv_init);
