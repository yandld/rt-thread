
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


rt_device_t cam0, cam1;

uint8_t cam0_buf0[320*240] __attribute__ ((aligned (4)));
uint8_t cam0_buf1[320*240] __attribute__ ((aligned (4)));

uint8_t cam1_buf0[320*240] __attribute__ ((aligned (4)));
uint8_t cam1_buf1[320*240] __attribute__ ((aligned (4)));

uint8_t cam0_swtich = 0;
uint8_t cam1_swtich = 0;

uint16_t  lcd_buf[320*240] __attribute__ ((aligned (32)));


uint32_t cam_start_xfer(rt_device_t dev, uint8_t *buf);
    
    
static void gray2rgb565(uint8_t *gray_buf, uint16_t* rgb565_buf, uint32_t len)
{
    for(int i=0; i<len; i++)
    {
     //   uint8_t byte = (gray_buf[i]>> 4) | ((gray_buf[i] & 0x0F)<<4);
     //   gray_buf[i] = byte;
        rgb565_buf[i] = ((gray_buf[i] >> 3) << 11) | ((gray_buf[i] >> 2) << 5) | (gray_buf[i] >> 3);
    }
    
}




static volatile uint8_t cam0_vsync_flag = 0;
static volatile uint8_t cam1_vsync_flag = 0;

rt_err_t cam0_rx_indicate(rt_device_t dev, rt_size_t size)
{
    cam0_vsync_flag = 1;
    return RT_EOK;
}

rt_err_t cam1_rx_indicate(rt_device_t dev, rt_size_t size)
{
    cam1_vsync_flag = 1;
    return RT_EOK;
}



int camera(void)
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

//    edma_channel_Preemption_config_t edma_channel_Preemption_config;
//    
//    edma_channel_Preemption_config.enableChannelPreemption = true;
//    edma_channel_Preemption_config.enablePreemptAbility = false;
//    edma_channel_Preemption_config.channelPriority = 4;

//    EDMA_SetChannelPreemptionConfig(DMA0, 0, &edma_channel_Preemption_config);
//    
//    edma_channel_Preemption_config.enableChannelPreemption = false;
//    edma_channel_Preemption_config.enablePreemptAbility = true;
//    edma_channel_Preemption_config.channelPriority = 1;

//    EDMA_SetChannelPreemptionConfig(DMA0, 2, &edma_channel_Preemption_config);
    
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

    edma_channel_Preemption_config_t edma_channel_Preemption_config;
        
    edma_channel_Preemption_config.enableChannelPreemption = false;
    edma_channel_Preemption_config.enablePreemptAbility = true;
    edma_channel_Preemption_config.channelPriority = 0;

    EDMA_SetChannelPreemptionConfig(DMA1, 1, &edma_channel_Preemption_config);
    
    edma_channel_Preemption_config.enableChannelPreemption = true;
    edma_channel_Preemption_config.enablePreemptAbility = false;
    edma_channel_Preemption_config.channelPriority = 1;
    EDMA_SetChannelPreemptionConfig(DMA1, 2, &edma_channel_Preemption_config);

    void smart_dma_g2rgb_init(void);
    void smart_dma_g2rgb_run(void *input, void *output, uint32_t size);
    
    smart_dma_g2rgb_init();
        
//    memset(cam1_buf0, 0xFF, 320*240);
//    smart_dma_g2rgb_run((!cam1_swtich)?(cam1_buf0):(cam1_buf1), lcd_buf, 320*240);
//    
//    rt_kprintf("%X %X %X %X\r\n", lcd_buf[0], lcd_buf[1], lcd_buf[2], lcd_buf[3]);
//    while(1)
//    {
//        
//    }
    
    
    while(1)
    {
        
        if(cam1_vsync_flag)
        {
            cam_start_xfer(cam1, (cam1_swtich)?(cam1_buf0):(cam1_buf1));
            
            smart_dma_g2rgb_run((!cam1_swtich)?(cam1_buf0):(cam1_buf1), lcd_buf, 320*240);
        //    gray2rgb565((!cam1_swtich)?(cam1_buf0):(cam1_buf1), lcd_buf, 320*240);
            cam1_swtich ^= 0x01;

            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 240;
            area.y2 = 240+240;

            LCD_SetWindow(&area);

            FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf, 320*240);
            while(WR_DMATransferDone == false)
            {
              //  rt_thread_mdelay(1);
            }
            
            rt_kprintf("cam1\r\n");
            
            cam1_vsync_flag = 0;
        }
        
        if(cam0_vsync_flag)
        {
            cam_start_xfer(cam0, (cam0_swtich)?(cam0_buf0):(cam0_buf1));
            smart_dma_g2rgb_run((!cam0_swtich)?(cam0_buf0):(cam0_buf1), lcd_buf, 320*240);
          //  gray2rgb565((!cam0_swtich)?(cam0_buf0):(cam0_buf1), lcd_buf, 320*240);
            cam0_swtich ^= 0x01;
            
            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 0;
            area.y2 = 240;

            LCD_SetWindow(&area);
            
            FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf, 320*240);

            while(WR_DMATransferDone == false)
            {
             //   rt_thread_mdelay(1);
            }

            rt_kprintf("cam0\r\n");
            cam0_vsync_flag = 0;
        }

      //  rt_thread_mdelay(1);
    }

}





#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(camera, dump memory trace for heap);
#endif
