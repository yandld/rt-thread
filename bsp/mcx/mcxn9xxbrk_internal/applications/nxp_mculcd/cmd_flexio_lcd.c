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
#include "fsl_flexio_mculcd.h"
#include "fsl_flexio_mculcd_edma.h"
#include "fsl_gpio.h"

#include "st7796.h"

#define GPIO_LCD_RS     (7)
#define GPIO_LCD_RST    (4*32+7)
#define GPIO_LCD_CS     (12)


#define LCD_FLEXIO          FLEXIO0
#define LCD_FLEXIO_FREQ     150000000
#define LCD_FLEXIO_BAUD     16000000


static FLEXIO_MCULCD_Type mculcd;


static void lcd_impl_set_cs_pin(bool set)
{
    if (set)
    {
        rt_pin_write(GPIO_LCD_CS, 1);
    } 
    else
    {
        rt_pin_write(GPIO_LCD_CS, 0);
    }
}

static void lcd_impl_set_rs_pin(bool set)
{
    if (set)
    {
        rt_pin_write(GPIO_LCD_RS, 1);
    }
    else
    {
        rt_pin_write(GPIO_LCD_RS, 0);
    }
}


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
    FLEXIO_MCULCD_StartTransfer(&mculcd);

    FLEXIO_MCULCD_WriteCommandBlocking(&mculcd, (uint32_t)cmd[0]);

    if (len > 1)
    {
        for (uint8_t i = 1; i < len; i++) 
        {
            uint16_t data = cmd[i]; /* Zero-extend */
            FLEXIO_MCULCD_WriteDataArrayBlocking(&mculcd, &data, 2);
        }
    }

    FLEXIO_MCULCD_StopTransfer(&mculcd);
    
    return ST7796_OK;
}

static st7796_ret_t spi_write_data_cb(void *handle, uint8_t *data, uint32_t len)
{
    FLEXIO_MCULCD_StartTransfer(&mculcd);
    FLEXIO_MCULCD_WriteDataArrayBlocking(&mculcd, data, len);
    FLEXIO_MCULCD_StopTransfer(&mculcd);
    
    return ST7796_OK;
}


 st7796_lcd_t s_lcd = 
{
    .config =
        {
            .direction = 0,
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

//static uint8_t buf[100*100*2];


void flexio_lcd_init(void)
{

    rt_pin_mode(GPIO_LCD_RST, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LCD_RS, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LCD_CS, PIN_MODE_OUTPUT);
    
    rt_pin_write(GPIO_LCD_CS, 1);
    rt_pin_write(GPIO_LCD_RS, 1);
    rt_pin_write(GPIO_LCD_RST, 1);
    
    CLOCK_AttachClk(kPLL0_to_FLEXIO);
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, CLOCK_GetPll0OutFreq() / LCD_FLEXIO_FREQ);

    flexio_mculcd_config_t flexio_cfg;
    FLEXIO_MCULCD_GetDefaultConfig(&flexio_cfg);

    flexio_cfg.baudRate_Bps  = LCD_FLEXIO_BAUD;
    flexio_cfg.enableInDebug = true;
    flexio_cfg.enableInDoze  = true;

    mculcd.flexioBase = LCD_FLEXIO;
    mculcd.busType    = kFLEXIO_MCULCD_8080;

    mculcd.timerIndex = 0U;

    mculcd.txShifterStartIndex = 0;
    mculcd.txShifterEndIndex   = 7;
    mculcd.rxShifterStartIndex = 0;
    mculcd.rxShifterEndIndex   = 7;

    mculcd.dataPinStartIndex = 16;
    mculcd.RDPinIndex        = 0;
    mculcd.ENWRPinIndex      = 1;

    mculcd.setCSPin = lcd_impl_set_cs_pin;
    mculcd.setRSPin = lcd_impl_set_rs_pin;
    
    rt_kprintf("flexio_lcd:%d\r\n", CLOCK_GetFlexioClkFreq());
    FLEXIO_MCULCD_Init(&mculcd, &flexio_cfg, CLOCK_GetFlexioClkFreq());
    st7796_lcd_init(&s_lcd);
}

int flexio_lcd(void)
{
    int i;

    flexio_lcd_init();
    
    while(1)
    {
      //  memset(buf, 0xFF, sizeof(buf));
     //   st7796_lcd_load(&s_lcd, buf, 10, 100, 10, 100);
        rt_thread_mdelay(200);
    //    memset(buf, 0x00, sizeof(buf));
     //   st7796_lcd_load(&s_lcd, buf, 10, 100, 10, 100);
    //    rt_thread_mdelay(200);
    }

    return RT_EOK;
}





#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(flexio_lcd, the flexio_lcd);
#endif


