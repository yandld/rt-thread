#include "drv_lcd_flexio.h"

#define GPIO_LCD_RS     (7)
#define GPIO_LCD_RST    (4*32+7)
#define GPIO_LCD_CS     (12)
#define GPIO_CAPT_INT   (4*32+6)


#define LCD_FLEXIO          FLEXIO0
#define LCD_FLEXIO_FREQ     150000000
#define LCD_FLEXIO_BAUD     16000000


static lcd_flexio_t lcd_flexio_obj;

static void lcd_impl_set_cs_pin(bool set)
{
    rt_pin_write(GPIO_LCD_CS, (set)?(1):(0));
}

static void lcd_impl_set_rs_pin(bool set)
{
    rt_pin_write(GPIO_LCD_RS, (set)?(1):(0));
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
    lcd_flexio_t *lcd_flexio = (lcd_flexio_t*)handle;
    
    FLEXIO_MCULCD_StartTransfer(&lcd_flexio_obj.mculcd);

    FLEXIO_MCULCD_WriteCommandBlocking(&lcd_flexio_obj.mculcd, (uint32_t)cmd[0]);

    if (len > 1)
    {
        for (uint8_t i = 1; i < len; i++) 
        {
            uint16_t data = cmd[i]; /* Zero-extend */
            FLEXIO_MCULCD_WriteDataArrayBlocking(&lcd_flexio_obj.mculcd, &data, 2);
        }
    }

    FLEXIO_MCULCD_StopTransfer(&lcd_flexio_obj.mculcd);
    
    return ST7796_OK;
}

static st7796_ret_t spi_write_data_cb(void *handle, uint8_t *data, uint32_t len)
{
    lcd_flexio_t *lcd_flexio = (lcd_flexio_t*)handle;
    
    FLEXIO_MCULCD_StartTransfer(&lcd_flexio_obj.mculcd);
    FLEXIO_MCULCD_WriteDataArrayBlocking(&lcd_flexio_obj.mculcd, data, len);
    FLEXIO_MCULCD_StopTransfer(&lcd_flexio_obj.mculcd);
    
    return ST7796_OK;
}


int drv_lcd_flexio_hw_init(void)
{
    /* INT PIN is used for I2C ADDR at power up of GP911 */
    rt_pin_mode(GPIO_CAPT_INT, PIN_MODE_OUTPUT);
    rt_pin_write(GPIO_CAPT_INT, 0);
    
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


    lcd_flexio_obj.mculcd.flexioBase = LCD_FLEXIO;
    lcd_flexio_obj.mculcd.busType    = kFLEXIO_MCULCD_8080;

    lcd_flexio_obj.mculcd.timerIndex = 0U;

    lcd_flexio_obj.mculcd.txShifterStartIndex = 0;
    lcd_flexio_obj.mculcd.txShifterEndIndex   = 7;
    lcd_flexio_obj.mculcd.rxShifterStartIndex = 0;
    lcd_flexio_obj.mculcd.rxShifterEndIndex   = 7;

    lcd_flexio_obj.mculcd.dataPinStartIndex = 16;
    lcd_flexio_obj.mculcd.RDPinIndex        = 0;
    lcd_flexio_obj.mculcd.ENWRPinIndex      = 1;

    lcd_flexio_obj.mculcd.setCSPin = lcd_impl_set_cs_pin;
    lcd_flexio_obj.mculcd.setRSPin = lcd_impl_set_rs_pin;
    
    FLEXIO_MCULCD_Init(&lcd_flexio_obj.mculcd, &flexio_cfg, CLOCK_GetFlexioClkFreq());
    
    lcd_flexio_obj.st7796.config.direction = ST7796_DIR_90;
    lcd_flexio_obj.st7796.config.pix_fmt = ST7796_RGB565;
    lcd_flexio_obj.st7796.config.bgr_mode = 1;
    lcd_flexio_obj.st7796.config.inversion = 0;
    lcd_flexio_obj.st7796.config.mirrored = 1;
    lcd_flexio_obj.st7796.cb.reset_cb = reset_cb;
    lcd_flexio_obj.st7796.cb.write_cmd_cb = spi_write_cmd_cb;
    lcd_flexio_obj.st7796.cb.write_data_cb = spi_write_data_cb;
    lcd_flexio_obj.st7796.user_data = &lcd_flexio_obj;
    lcd_flexio_obj.parent.user_data = &lcd_flexio_obj;
    
    st7796_lcd_init(&lcd_flexio_obj.st7796);
    
    rt_device_register(&lcd_flexio_obj.parent, "lcd", RT_DEVICE_FLAG_RDWR);
        
    return RT_EOK;
}

INIT_DEVICE_EXPORT(drv_lcd_flexio_hw_init);



#define TEST_LCD_X  (60)
#define TEST_LCD_Y  (60)
#define RGB565_RED     0xF800
#define RGB565_GREEN   0x07E0
#define RGB565_BLUE    0x001F

static int lcd_flexio_test(void)
{
    int i;
    
    uint16_t *buf = rt_malloc(TEST_LCD_X*TEST_LCD_Y*sizeof(uint16_t));
    
    RT_ASSERT(buf != RT_NULL);

    rt_device_t lcd = rt_device_find("lcd");
    lcd_flexio_t *lcd_flexio = (lcd_flexio_t*)lcd->user_data;
    
    while(1)
    {
        for(i=0; i<TEST_LCD_X*TEST_LCD_Y; i++) buf[i] = RGB565_BLUE;
        st7796_lcd_load(&lcd_flexio_obj.st7796, (uint8_t*)buf, 0, TEST_LCD_X, 0, TEST_LCD_Y);
        rt_thread_mdelay(200);
        for(i=0; i<TEST_LCD_X*TEST_LCD_Y; i++) buf[i] = RGB565_GREEN;
        st7796_lcd_load(&lcd_flexio_obj.st7796, (uint8_t*)buf, 0, TEST_LCD_X, 0, TEST_LCD_Y);
        rt_thread_mdelay(200);
    }

    return RT_EOK;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(lcd_flexio_test, the lcd flexio test);
#endif


