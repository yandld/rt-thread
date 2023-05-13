#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>



#include "app_drv_can.h"


static void app_can_event_cb(uint32_t id, uint8_t *buf, uint8_t len)
{
    int i;
    rt_kprintf("id:0x%X, len:%d\r\n", id, len);

    for (i = 0; i < len; i++)
    {
        rt_kprintf("%02X ", buf[i]);
    }

    rt_kprintf("\r\n");
}

int can_test(void)
{
    int i;

    app_can_init(1000*1000, 2*1000*1000, 0);
    app_can_add_event_cb(app_can_event_cb, 0, NULL);
    
    uint8_t tx_data[64];
    
    for(i=0; i<64; i++)
    {
        tx_data[i] = i;
    }


    while (1)
    {
        app_send(tx_data, 0x123, 8);
        rt_thread_mdelay(1);
    }
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(can_test, the can test);
#endif
