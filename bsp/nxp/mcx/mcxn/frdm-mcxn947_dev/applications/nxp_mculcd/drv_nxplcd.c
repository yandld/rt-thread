#include "drv_nxplcd.h"


#define GPIO_LCD_RS     (7)
#define GPIO_LCD_RST    (4*32+7)
#define GPIO_LCD_CS     (12)
#define GPIO_CAPT_INT   (4*32+6)
#define GPIO_LCD_DC     (1*32+2)

#define LCD_FLEXIO          FLEXIO0
#define LCD_FLEXIO_BAUD     24000000
#define LCD_SPI_DEV_NAME    ("spi3")


static nxplcd_t lcd_flexio_obj;

       
static st7796_ret_t reset_cb(void *handle)
{
    /* INT PIN is used for I2C ADDR at power up of GP911 */
    rt_pin_mode(GPIO_CAPT_INT, PIN_MODE_OUTPUT);
    rt_pin_write(GPIO_CAPT_INT, 0);
    
    rt_pin_write(GPIO_LCD_RST, 0);
    rt_thread_mdelay(10);
    rt_pin_write(GPIO_LCD_RST, 1);
    rt_thread_mdelay(10);
    
    rt_pin_mode(GPIO_CAPT_INT, PIN_MODE_INPUT);
    
    return ST7796_OK;
}

static st7796_ret_t backlight_cb(void *handle, uint8_t on)
{
    return ST7796_OK;
}



static void lcd_impl_set_cs_pin(bool set)
{
    rt_pin_write(GPIO_LCD_CS, (set)?(1):(0));
}

static void lcd_impl_set_rs_pin(bool set)
{
    rt_pin_write(GPIO_LCD_RS, (set)?(1):(0));
}

static void flexio_lcd_dma_callback(FLEXIO_MCULCD_Type *base, flexio_mculcd_edma_handle_t *handle, status_t status, void *userData)
{
    nxplcd_t *nxplcd = (nxplcd_t *)userData;
    rt_sem_release(nxplcd->sem);
}

static st7796_ret_t flexio_write_cmd_cb(void *handle, uint8_t *cmd, uint8_t len)
{
    nxplcd_t *nxplcd = (nxplcd_t*)handle;
    
#if LCD_FLEXIO_USING_EDMA
    rt_sem_take(nxplcd->sem, RT_WAITING_FOREVER);
#endif
    
    FLEXIO_MCULCD_StartTransfer(&nxplcd->mculcd);
    FLEXIO_MCULCD_WriteCommandBlocking(&nxplcd->mculcd, (uint32_t)cmd[0]);

    if (len > 1)
    {
        for (uint8_t i = 1; i < len; i++) 
        {
            uint16_t data = cmd[i]; /* Zero-extend */
            FLEXIO_MCULCD_WriteDataArrayBlocking(&nxplcd->mculcd, &data, 2);
        }
    }

    FLEXIO_MCULCD_StopTransfer(&nxplcd->mculcd);
    
#if LCD_FLEXIO_USING_EDMA
    rt_sem_release(nxplcd->sem);
#endif
    
    return ST7796_OK;
}

static st7796_ret_t flexio_write_data_cb(void *handle, const uint8_t *data, uint32_t len)
{
    nxplcd_t *nxplcd = (nxplcd_t*)handle;
    
#if LCD_FLEXIO_USING_EDMA
    rt_sem_take(nxplcd->sem, RT_WAITING_FOREVER);
    FLEXIO_MCULCD_WriteDataEDMA(&nxplcd->mculcd, &nxplcd->dma_hdl, data, len);

#else
    FLEXIO_MCULCD_StartTransfer(&nxplcd->mculcd);
    FLEXIO_MCULCD_WriteDataArrayBlocking(&nxplcd->mculcd, data, len);
    FLEXIO_MCULCD_StopTransfer(&nxplcd->mculcd);
#endif

    return ST7796_OK;
}

static st7796_ret_t spi_write_cmd_cb(void *handle, uint8_t *cmd, uint8_t len)
{
    nxplcd_t *nxplcd = (nxplcd_t*)handle;
    rt_pin_write(GPIO_LCD_DC, 0);
    rt_spi_send(nxplcd->spi_dev, cmd, 1);
    
    if (len > 1)
    {
        rt_pin_write(GPIO_LCD_DC, 1);
        rt_spi_send(nxplcd->spi_dev, &cmd[1], len-1);
    }

    return ST7796_OK;
}

static st7796_ret_t spi_write_data_cb(void *handle, uint8_t *data, uint32_t len)
{
    nxplcd_t *nxplcd = (nxplcd_t*)handle;
    rt_pin_write(GPIO_LCD_DC, 1);
    rt_spi_send(nxplcd->spi_dev, data, len);
    return ST7796_OK;
}
                           

int drv_nxplcd_init(void)
{
    rt_pin_mode(GPIO_LCD_RST, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LCD_RS, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LCD_CS, PIN_MODE_OUTPUT);
    
    rt_pin_write(GPIO_LCD_CS, 1);
    rt_pin_write(GPIO_LCD_RS, 1);
    

    CLOCK_AttachClk(kPLL0_to_FLEXIO);
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, CLOCK_GetPll0OutFreq() / CLOCK_GetCoreSysClkFreq());

    lcd_flexio_obj.DMAX = DMA0;
    lcd_flexio_obj.sem = rt_sem_create("sem_lcd", 1, RT_IPC_FLAG_FIFO);
    
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
    
    lcd_flexio_obj.st7796.config.direction = ST7796_DIR_270;
    lcd_flexio_obj.st7796.config.pix_fmt = ST7796_RGB565;
    lcd_flexio_obj.st7796.config.bgr_mode = 1;
    lcd_flexio_obj.st7796.config.inversion = 0;
    lcd_flexio_obj.st7796.config.mirrored = 1;
    lcd_flexio_obj.st7796.cb.reset_cb = reset_cb;
    
#if LCD_FLEXIO_USING_SPI
    lcd_flexio_obj.st7796.cb.write_cmd_cb = spi_write_cmd_cb;
    lcd_flexio_obj.st7796.cb.write_data_cb = spi_write_data_cb;
#else
    lcd_flexio_obj.st7796.cb.write_cmd_cb = flexio_write_cmd_cb;
    lcd_flexio_obj.st7796.cb.write_data_cb = flexio_write_data_cb;
#endif
    lcd_flexio_obj.st7796.user_data = lcd_flexio_obj.parent.user_data = &lcd_flexio_obj;
    
#if LCD_FLEXIO_USING_EDMA

    EDMA_EnableChannelRequest(lcd_flexio_obj.DMAX, 0);
    EDMA_SetChannelMux(lcd_flexio_obj.DMAX, 0, kDmaRequestMuxFlexIO0ShiftRegister0Request);
    
    EDMA_CreateHandle(&lcd_flexio_obj.tx_handle, lcd_flexio_obj.DMAX, 0);
    FLEXIO_MCULCD_TransferCreateHandleEDMA(&lcd_flexio_obj.mculcd, &lcd_flexio_obj.dma_hdl, flexio_lcd_dma_callback, (void *)&lcd_flexio_obj, &lcd_flexio_obj.tx_handle, NULL);
                                           
#endif
    
#if LCD_FLEXIO_USING_SPI
    rt_pin_mode(GPIO_LCD_DC, PIN_MODE_OUTPUT);
    rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_uint32_t pin);
    rt_hw_spi_device_attach(LCD_SPI_DEV_NAME, "spid_lcd", GPIO_LCD_CS);
    lcd_flexio_obj.spi_dev = (struct rt_spi_device *)rt_device_find("spid_lcd");
    
#endif
    
    st7796_lcd_init(&lcd_flexio_obj.st7796);
    rt_device_register(&lcd_flexio_obj.parent, "lcd", RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}

INIT_DEVICE_EXPORT(drv_nxplcd_init);





int nxplcd_test(void)
{
    int i;
    rt_device_t lcd = rt_device_find("lcd");
    nxplcd_t *lcd_flexio = (nxplcd_t*)lcd->user_data;
    uint16_t *buf = rt_malloc(480*320*2);
    
    
    
    rt_kprintf("test lcd\r\n");
    
    while(1)
    {
        rt_memset(buf, 0xFF, 480*320*2);
        st7796_lcd_load(&lcd_flexio->st7796, buf, 0, 480-1, 0, 320-1);
        rt_thread_mdelay(200);
    }

}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(nxplcd_test, nxplcd_test);
#endif


