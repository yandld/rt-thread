#include <lvgl.h>
#include <rtdevice.h>
#include "drv_nxplcd_capt.h"



static void tp_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    bool found_track = false;
    
    gt911_input_t ctp_input = {0};

    capt_t *capt = (capt_t*)drv->user_data;
    
    if(!capt)
    {
        rt_kprintf("no capt device\r\n");
        return;
    }

    gt911_ctp_read(&capt->gt911, &ctp_input);

    for (uint8_t i = 0; i < ctp_input.num_pos; i++) 
    {
        if (ctp_input.pos[i].id == 0)
        {
            data->state   = LV_INDEV_STATE_PRESSED;
            data->point.x = capt->gt911.pos_y_max - ctp_input.pos[i].pos_y;
            data->point.y = ctp_input.pos[i].pos_x;
            
            found_track = true;
           // rt_kprintf("x:%d, y:%d\r\n", capt->gt911.pos_y_max - ctp_input.pos[i].pos_y, ctp_input.pos[i].pos_x);
        }
    }
    
    /* No track #0 found... */
    if (!found_track) data->state = LV_INDEV_STATE_RELEASED;
}

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
    
    rt_device_t dev = rt_device_find("capt");
    
    RT_ASSERT(dev != RT_NULL);
    
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = tp_read;
    indev_drv.user_data = dev->user_data;

    lv_indev_drv_register(&indev_drv);
}
