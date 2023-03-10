#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#include "stdlib.h"
#include "fsl_device_registers.h"
#include "fsl_lpspi.h"
#include "pin_mux.h"

#include "clock_config.h"
#include "flexio_8080_drv.h"
#include "lcd_ssd1963_drv.h"
#include "zoo_rotate.h"
#include "fsl_lpspi_edma.h"
#include "fsl_lpspi.h"


void smart_dma_g2rgb_init(void);
void smart_dma_g2rgb_run(void *input, void *output, uint32_t size);
    
rt_device_t cam0, cam1;

uint8_t cam0_buf0[320*240] __attribute__ ((aligned (4)));
uint8_t cam0_buf1[320*240] __attribute__ ((aligned (4)));

uint8_t cam1_buf0[320*240] __attribute__ ((aligned (4)));
uint8_t cam1_buf1[320*240] __attribute__ ((aligned (4)));

uint8_t cam0_swtich = 0;
uint8_t cam1_swtich = 0;

uint16_t  lcd_buf[320*240] __attribute__ ((aligned (32)));

uint32_t cam_start_xfer(rt_device_t dev, uint8_t *buf);
    
    
//static void gray2rgb565(uint8_t *gray_buf, uint16_t* rgb565_buf, uint32_t len)
//{
//    for(int i=0; i<len; i++)
//    {
//     //   uint8_t byte = (gray_buf[i]>> 4) | ((gray_buf[i] & 0x0F)<<4);
//     //   gray_buf[i] = byte;
//        rgb565_buf[i] = ((gray_buf[i] >> 3) << 11) | ((gray_buf[i] >> 2) << 5) | (gray_buf[i] >> 3);
//    }
//    
//}


#define LEDB_PIN        ((3*32)+4)

static rt_sem_t cam0_sem, cam1_sem;


rt_err_t cam0_rx_indicate(rt_device_t dev, rt_size_t size)
{    
    rt_sem_release(cam0_sem);
    return RT_EOK;
}

rt_err_t cam1_rx_indicate(rt_device_t dev, rt_size_t size)
{
    rt_sem_release(cam1_sem);
    return RT_EOK;
}


static void camera_thread_entry(void *parameter)
{
    int i;
    
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 3U); /* 150MHz / 3 = 50MHz */
    
    /* SPI */
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM3);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1U);
    
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM4);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1U);
    
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM5);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1U);
    
    /* init I2C0 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);
    CLOCK_EnableClock(kCLOCK_LPFlexComm0);
    CLOCK_EnableClock(kCLOCK_LPI2c0);
    
    
    rt_kprintf("CoreClock:%d\r\n", CLOCK_GetCoreSysClkFreq());
    rt_kprintf("FROHF:%d\r\n", CLOCK_GetFreq(kCLOCK_FroHf));
    rt_kprintf("FLCOMM0:%d\r\n", CLOCK_GetLPFlexCommClkFreq(0));
    rt_kprintf("FLCOMM3:%d\r\n", CLOCK_GetLPFlexCommClkFreq(3));
    rt_kprintf("FLCOMM4:%d\r\n", CLOCK_GetLPFlexCommClkFreq(4));

    
    /* Init FlexIO for this demo. */
    Demo_FLEXIO_8080_Init();
    LCD_ST7796S_Init();
    for(i=0; i<5; i++)
    {
        LCD_FillColorWhole(Red);
        SDK_DelayAtLeastUs(1000*100, CLOCK_GetCoreSysClkFreq());
        LCD_FillColorWhole(Blue);
        SDK_DelayAtLeastUs(1000*100, CLOCK_GetCoreSysClkFreq());
    }


    cam0_sem = rt_sem_create("cam0_sem", 0, RT_IPC_FLAG_FIFO);
    cam1_sem = rt_sem_create("cam1_sem", 0, RT_IPC_FLAG_FIFO);
    
    AreaPoints_t area;

    cam0 = rt_device_find("cam0");
    if(cam0)
    {
        cam0->rx_indicate = cam0_rx_indicate;
        rt_device_open(cam0, RT_DEVICE_OFLAG_RDWR);
        cam_start_xfer(cam0, cam0_buf0);
    }

    
    cam1 = rt_device_find("cam1");
    if(cam1)
    {
        cam1->rx_indicate = cam1_rx_indicate;
        rt_device_open(cam1, RT_DEVICE_OFLAG_RDWR);
        cam_start_xfer(cam1, cam1_buf0);
    }

    
    smart_dma_g2rgb_init();
 //   SYSCON->AHBMATPRIO = 1<<10;
    
    while(1)
    {
        static uint8_t led_switch = 0;
            
        led_switch ^= 0x01;
        rt_pin_write(LEDB_PIN, led_switch);    /* Set GPIO output 1 */
        
        if(rt_sem_take(cam0_sem, rt_tick_from_millisecond(100)) == RT_EOK)
        {
            cam_start_xfer(cam1, (cam1_swtich)?(cam1_buf0):(cam1_buf1));
            smart_dma_g2rgb_run((!cam1_swtich)?(cam1_buf0):(cam1_buf1), lcd_buf, 320*240);
            cam1_swtich ^= 0x01;

            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 240;
            area.y2 = 240+240;

            LCD_SetWindow(&area);

            FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf, 320*240);
            while(WR_DMATransferDone == false)
            {
                rt_thread_mdelay(1);
            }
            
         //   rt_kprintf("cam1\r\n");
        }

        
        if(rt_sem_take(cam1_sem, rt_tick_from_millisecond(100)) == RT_EOK)
        {
            cam_start_xfer(cam0, (cam0_swtich)?(cam0_buf0):(cam0_buf1));
            smart_dma_g2rgb_run((!cam0_swtich)?(cam0_buf0):(cam0_buf1), lcd_buf, 320*240);
            cam0_swtich ^= 0x01;
            
            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 0;
            area.y2 = 240;

            LCD_SetWindow(&area);
            
            FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf, 320*240);

            while(WR_DMATransferDone == false)
            {
                rt_thread_mdelay(1);
            }

          //  rt_kprintf("cam0\r\n");
        }
    }
}



int camera(void)
{
    rt_thread_t tid = rt_thread_find("tcam");
    
    if(tid == RT_NULL)
    {
        tid = rt_thread_create("tcam", camera_thread_entry, RT_NULL, 2048, 5, 20);
        rt_thread_startup(tid);
    }
    else
    {
        rt_kprintf("thread alread exist\r\n");
    }

    return RT_EOK;
}





#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(camera, the dual camera demo);
#endif
