#include <lvgl.h>
#include "drv_nxplcd.h"

void lv_demo_benchmark(void);

#define CAM_W   (160)
#define CAM_H   (120)

extern uint16_t lcd_buf[];
extern int diff_sum;
extern int frame_cnt;

 lv_img_dsc_t img_video = 
 {
  .header.always_zero = 0,
  .header.w = CAM_W,
  .header.h = CAM_H,
  .data_size = CAM_W*CAM_H*2,
  .header.cf = LV_IMG_CF_TRUE_COLOR,
  .data = (const uint8_t*)lcd_buf,
};


typedef struct
{
    lv_obj_t *img;
    lv_obj_t *led2;
    lv_obj_t *label_log;
    lv_obj_t *title_bar;
    lv_obj_t *slider;
    lv_obj_t *panel;
    lv_obj_t *slider_label;
    lv_obj_t *dd;
    lv_style_t style_panel;
    lv_style_t style_log;

    lv_timer_t *timer_blink;
}ui_t;

static ui_t ui;
uint8_t cam_idx = 0;


static void app_zoom(int val)
{
    int w = CAM_W;
    int h = CAM_H;
    
    lv_img_set_zoom(ui.img, 256 + val);
    lv_obj_set_size(ui.panel, w+14+val/1.5, h+14+val/2);
}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "size: %d%%", (int)lv_slider_get_value(slider));
    lv_label_set_text(ui.slider_label, buf);
    lv_obj_align_to(ui.slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);
    
    app_zoom(lv_slider_get_value(lv_event_get_target(e)));
}

static void timer_video_callback(lv_timer_t* timer)
{
    lv_img_set_src(ui.img, &img_video);
    
    if(frame_cnt > 20)
    {
        if(diff_sum < 3000)
        {
            lv_style_set_bg_color(&ui.style_panel, lv_palette_main(LV_PALETTE_GREEN));
            lv_timer_set_period(ui.timer_blink, 200);
            lv_label_set_text(ui.label_log, "#000000 NO MOTION#");

        }
        else
        {
            lv_style_set_bg_color(&ui.style_panel, lv_palette_main(LV_PALETTE_RED));
            lv_label_set_text(ui.label_log, "#ff0000 MOTION DETECTED! #");
            lv_timer_set_period(ui.timer_blink, 30);
        }
    }

    
}

static void timer_blink_callback(lv_timer_t* timer)
{
    static int count = 0;
    static bool is_increasing = true;

    int opa_table[] = {LV_OPA_0, LV_OPA_10, LV_OPA_20, LV_OPA_30, LV_OPA_40, LV_OPA_50, LV_OPA_60, LV_OPA_70, LV_OPA_80, LV_OPA_90, LV_OPA_100, LV_OPA_90, LV_OPA_80, LV_OPA_70, LV_OPA_60, LV_OPA_50, LV_OPA_40, LV_OPA_30, LV_OPA_20, LV_OPA_10};
    if (is_increasing)
    {
        count++;
        if (count >= ARRAY_SIZE(opa_table))
        {
            is_increasing = false;
        }
    }
    else 
    {
        count--;
        if (count <= 0)
        {
            is_increasing = true;
        }
    }
    
  //  lv_style_set_shadow_opa(&style, opa_table[count]);
    //lv_style_set_border_opa(&style, opa_table[count]);
    lv_style_set_bg_opa(&ui.style_panel, opa_table[count]);
  //  lv_style_set_outline_opa(&style, opa_table[count]);
    


}


static void dd_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED)
    {
        cam_idx = lv_dropdown_get_selected(obj);
        frame_cnt = 0;
    }
}


void lv_user_gui_init(void)
{
    int eb_cam(void);
    eb_cam();
    
    /* panel */
    ui.panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(ui.panel, CAM_W+10, CAM_H+10);
    lv_obj_align(ui.panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(ui.panel, &ui.style_panel, 0);
    
    lv_style_set_radius(&ui.style_panel, 5);
    lv_style_set_bg_color(&ui.style_panel, lv_palette_main(LV_PALETTE_GREEN));
    
    /* img */
    ui.img = lv_img_create(ui.panel);
    lv_obj_align(ui.img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(ui.img, LV_SIZE_CONTENT);
    lv_img_set_src(ui.img, &img_video);
    
    /* title_bar */
    ui.title_bar = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(ui.title_bar , LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ui.title_bar , LV_SIZE_CONTENT);
    lv_label_set_text(ui.title_bar , "NXP MCXN947 BRK MOTION DETECT CAMERA");
    lv_obj_align(ui.title_bar , LV_ALIGN_TOP_MID, 0, 5);
    

    /* label log */
    ui.label_log  = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(ui.label_log , LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(ui.label_log , LV_SIZE_CONTENT);
    lv_obj_align(ui.label_log , LV_ALIGN_TOP_MID, 0, 32);
    lv_label_set_text(ui.label_log, "#000000 NO MOTION#");
    lv_label_set_recolor(ui.label_log, true);
    
    lv_style_init(&ui.style_log);
    lv_style_set_text_font(&ui.style_log, &lv_font_montserrat_24);
    lv_obj_add_style(ui.label_log, &ui.style_log, 0);
    
    
    /*Create a slider in the center of the display*/
    ui.slider = lv_slider_create(lv_scr_act());
    lv_obj_align(ui.slider, LV_ALIGN_BOTTOM_MID, 0, -23);
    lv_obj_set_width(ui.slider , 220);
    lv_obj_add_event_cb(ui.slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_slider_set_value(ui.slider, 100, 0);
    app_zoom(lv_slider_get_value(ui.slider));
    
    /*Create a label below the slider*/
    ui.slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(ui.slider_label, "size: 100%");
    lv_obj_align_to(ui.slider_label, ui.slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 3);
    

    /* dd */
    ui.dd = lv_dropdown_create(lv_scr_act());
    lv_obj_align(ui.dd, LV_ALIGN_TOP_LEFT, 0, 70);
    lv_obj_set_width(ui.dd , 110);
    lv_dropdown_set_options(ui.dd, "CAMERA1\nCAMERA2");
    lv_obj_add_event_cb(ui.dd, dd_event_handler, LV_EVENT_ALL, NULL);
    
    /* timer */
    lv_timer_create(timer_video_callback, 50, NULL);
    ui.timer_blink = lv_timer_create(timer_blink_callback, 100, NULL);
}



