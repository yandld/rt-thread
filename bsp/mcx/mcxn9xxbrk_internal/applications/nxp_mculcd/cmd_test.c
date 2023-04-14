#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#include "stdlib.h"
#include "fsl_device_registers.h"
#include "fsl_lpadc.h"
#include "pin_mux.h"

#include "clock_config.h"
#include "fsl_lpadc.h"
#include "fsl_spc.h"
#include "fsl_common.h"
#include "fsl_ctimer.h"

#include "fsl_clock.h"
#include "fsl_gpio.h"

#include "st7796.h"

#define GPIO_LCD_DC     (1*32+2)
#define GPIO_LCD_RST    (3*32+1)
#define GPIO_LCD_CS     (1*32+3)



static struct rt_spi_device *spi_dev;
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_uint32_t pin);



static st7796_ret_t reset_cb(void *handle)
{
    rt_pin_write(GPIO_LCD_RST, 0);
    rt_thread_mdelay(10);
    rt_pin_write(GPIO_LCD_RST, 1);
    rt_thread_mdelay(10);
    return ST7796_OK;
}

static st7796_ret_t backlight_cb(void *handle, uint8_t on)
{
    return ST7796_OK;
}

static st7796_ret_t spi_write_cmd_cb(void *handle, uint8_t *cmd, uint8_t len)
{
    rt_pin_write(GPIO_LCD_DC, 0);
    rt_spi_send(spi_dev, cmd, 1);
    
    if (len > 1)
    {
        rt_pin_write(GPIO_LCD_DC, 1);
        rt_spi_send(spi_dev, &cmd[1], len-1);
    }

    return ST7796_OK;
}

static st7796_ret_t spi_write_data_cb(void *handle, uint8_t *data, uint32_t len)
{
    rt_pin_write(GPIO_LCD_DC, 1);
    rt_spi_send(spi_dev, data, len);
    return ST7796_OK;
}


static st7796_lcd_t s_lcd = 
{
    .config =
        {
            .direction = ST7796_DIR_90,
            .pix_fmt   = ST7796_RGB565,
            .bgr_mode  = 1,
            .inversion = 0,
            .mirrored  = 1,
        },
    .cb =
        {
            .reset_cb      = reset_cb,
            .write_cmd_cb  = spi_write_cmd_cb,
            .write_data_cb = spi_write_data_cb,
        },
    .user_data = RT_NULL,
};

static uint8_t buf[20*20*2];

int cmd_test(void)
{
    int i;

    rt_pin_mode(GPIO_LCD_RST, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LCD_DC, PIN_MODE_OUTPUT);
    
    rt_hw_spi_device_attach("spi3", "spi30", GPIO_LCD_CS);
    
    spi_dev = (struct rt_spi_device*)rt_device_find("spi30");
   
    st7796_lcd_init(&s_lcd);
    
    while(1)
    {
        memset(buf, 0xFF, sizeof(buf));
        st7796_lcd_load(&s_lcd, buf, 10, 20, 10, 20);
        rt_thread_mdelay(200);
        memset(buf, 0x00, sizeof(buf));
        st7796_lcd_load(&s_lcd, buf, 10, 20, 10, 20);
        rt_thread_mdelay(200);
    }

    return RT_EOK;
}





#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(cmd_test, the cmd_test);
#endif


