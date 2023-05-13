/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-18     Meco Man     The first version
 */
#include <lvgl.h>
#include "drv_nxplcd.h"

#define MY_DISP_HOR_RES     LCD_W
#define DISP_BUFFER_LINES   (LCD_H/4)


static nxplcd_t *lcd_flexio = RT_NULL;

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Descriptor of a display driver*/
static lv_disp_drv_t disp_drv;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[MY_DISP_HOR_RES * DISP_BUFFER_LINES];
static lv_color_t buf_2[MY_DISP_HOR_RES * DISP_BUFFER_LINES];

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    st7796_lcd_load(&lcd_flexio->st7796, (uint8_t*)color_p, area->x1, area->x2, area->y1, area->y2);
    lv_disp_flush_ready(disp_drv);
}

void lv_port_disp_init(void)
{
    rt_device_t lcd = rt_device_find("lcd");
    lcd_flexio = (nxplcd_t*)lcd->user_data;
    
    /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
    lv_disp_draw_buf_init(&disp_buf, buf_1, buf_2, MY_DISP_HOR_RES * DISP_BUFFER_LINES);

    lv_disp_drv_init(&disp_drv); /*Basic initialization*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = LCD_W;
    disp_drv.ver_res = LCD_H;

    disp_drv.rotated = LV_DISP_ROT_180;
    disp_drv.sw_rotate = 1;
    
    /*Set a display buffer*/
    disp_drv.draw_buf = &disp_buf; 


    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}
