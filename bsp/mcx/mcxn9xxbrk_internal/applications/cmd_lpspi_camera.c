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

#define CAM_RST_PIN     (1*32+19)

void smart_dma_g2rgb_init(void);
void smart_dma_g2rgb_run(void *input, void *output, uint32_t size);
    
rt_device_t cam0, cam1;

uint8_t cam0_buf[320*240] __attribute__ ((aligned (4)));
uint8_t cam1_buf[320*240] __attribute__ ((aligned (4)));

uint8_t display_swtich = 0;
uint16_t  lcd_buf[320*240*2] __attribute__ ((aligned (32)));
uint32_t last_time = 0;

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
    
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM5);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1U);
    
//    /* init I2C0 */
//    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u);
//    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);
//    CLOCK_EnableClock(kCLOCK_LPFlexComm0);
//    CLOCK_EnableClock(kCLOCK_LPI2c0);
    
    /* reset */
    rt_pin_mode(CAM_RST_PIN, PIN_MODE_OUTPUT); 
    rt_pin_write(CAM_RST_PIN, 0); 
    rt_thread_mdelay(1);
    rt_pin_write(CAM_RST_PIN, 1); 
    
    /* Init FlexIO for this demo. */
    Demo_FLEXIO_8080_Init();
    LCD_ST7796S_Init();
    for(i=0; i<5; i++)
    {
        LCD_FillColorWhole(Red);
        SDK_DelayAtLeastUs(1000*20, CLOCK_GetCoreSysClkFreq());
        LCD_FillColorWhole(Blue);
        SDK_DelayAtLeastUs(1000*20, CLOCK_GetCoreSysClkFreq());
    }

    cam0_sem = rt_sem_create("cam0_sem", 0, RT_IPC_FLAG_FIFO);
    cam1_sem = rt_sem_create("cam1_sem", 0, RT_IPC_FLAG_FIFO);
    
    AreaPoints_t area;
    smart_dma_g2rgb_init();
    
    cam0 = rt_device_find("cam0");
    if(cam0)
    {
        cam0->rx_indicate = cam0_rx_indicate;
        rt_device_open(cam0, RT_DEVICE_OFLAG_RDWR);
    }

    
    cam1 = rt_device_find("cam1");
    if(cam1)
    {
        cam1->rx_indicate = cam1_rx_indicate;
        rt_device_open(cam1, RT_DEVICE_OFLAG_RDWR);
    }

    SYSCON->AHBMATPRIO |= SYSCON_AHBMATPRIO_DMA1_MASK;
    
    while(1)
    {
        if(rt_sem_take(cam0_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            smart_dma_g2rgb_run(cam0_buf, lcd_buf, 320*240);
            cam_start_xfer(cam0, cam0_buf);
            
            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 0;
            area.y2 = 240;
            
            if(WR_DMATransferDone == true && display_swtich == 1 || (rt_tick_get_millisecond() - last_time > 1000))
            {
                LCD_SetWindow(&area);
                FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf, 320*(240*1));
                display_swtich = 0;
                last_time = rt_tick_get_millisecond();
                rt_kprintf("cam0\r\n");
            }
            
        }

        if(rt_sem_take(cam1_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            smart_dma_g2rgb_run(cam1_buf, lcd_buf+(320*240*1), 320*240);
            cam_start_xfer(cam1, cam1_buf);

            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 240;
            area.y2 = 240+240;
                        
            if(WR_DMATransferDone == true && display_swtich == 0 || (rt_tick_get_millisecond() - last_time > 1000))
            {
                LCD_SetWindow(&area);
                FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf+(320*240*1), 320*(240*1));
                display_swtich = 1;
                last_time = rt_tick_get_millisecond();
                rt_kprintf("cam1\r\n");
            }
            
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
