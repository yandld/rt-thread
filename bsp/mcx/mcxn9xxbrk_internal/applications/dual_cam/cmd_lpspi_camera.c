#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#include "stdlib.h"
#include "fsl_device_registers.h"
#include "fsl_lpspi.h"
#include "pin_mux.h"

#include "clock_config.h"
#include "fsl_lpspi_edma.h"
#include "fsl_lpspi.h"
#include "st7796.h"

#define CAM_RST_PIN     (1*32+19)

void smart_dma_g2rgb_init(void);
void smart_dma_g2rgb_run(void *input, void *output, uint32_t size);
    
rt_device_t cam0, cam1;

uint8_t cam0_buf[320*240] __attribute__ ((section(".ARM.__at_0x04000000")));
uint8_t cam1_buf[320*240] __attribute__ ((aligned (4)));

uint16_t  lcd_buf[320*240] __attribute__ ((aligned (32)));

uint32_t cam_start_xfer(rt_device_t dev, uint8_t *buf);
int rt_hw_hm0360_init(void);
    
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

extern  st7796_lcd_t s_lcd;

static void cam1_thread_entry(void *parameter)
{
    rt_hw_hm0360_init();
        
    //CLOCK_SetClkDiv(kCLOCK_DivPllClk, 2U); /* 150MHz / 3 = 50MHz */
        
    /* SPI */
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM3);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1U);
        
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM5);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1U);
        
    /* reset */
    rt_pin_mode(CAM_RST_PIN, PIN_MODE_OUTPUT); 
    rt_pin_write(CAM_RST_PIN, 0); 
    rt_thread_mdelay(1);
    rt_pin_write(CAM_RST_PIN, 1); 
    rt_thread_mdelay(1);

    cam0 = rt_device_find("cam0");
    if(cam0)
    {
        cam0_sem = rt_sem_create("cam0_sem", 0, RT_IPC_FLAG_FIFO);
        cam0->rx_indicate = cam0_rx_indicate;
        rt_device_open(cam0, RT_DEVICE_OFLAG_RDWR);
    }

    cam1 = rt_device_find("cam1");
    if(cam1)
    {
        cam1_sem = rt_sem_create("cam1_sem", 0, RT_IPC_FLAG_FIFO);
        cam1->rx_indicate = cam1_rx_indicate;
        rt_device_open(cam1, RT_DEVICE_OFLAG_RDWR);
    }

        
    while(1)
    {
        if(rt_sem_take(cam0_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            cam_start_xfer(cam0, cam0_buf);
         //   rt_kprintf("cam0\r\n");
        }
        
        if(rt_sem_take(cam1_sem, 0) == RT_EOK)
        {
            cam_start_xfer(cam1, cam1_buf);
         //   rt_kprintf("cam1\r\n");
        }
    }
}

static void cam_lcd_thread_entry(void *parameter)
{
    int i;
    uint8_t display_swtich = 0;
    
    smart_dma_g2rgb_init();
    
    for(i=0; i<320*240; i++)
    {
        lcd_buf[i] = 0x0000;
    }
    
    st7796_lcd_load(&s_lcd, (uint8_t*)lcd_buf, 0, 320, 0, 240-1);
    for(i=0; i<320*240; i++)
    {
        lcd_buf[i] = 0x0000;
    }
    st7796_lcd_load(&s_lcd, (uint8_t*)lcd_buf, 0, 320, 240, 240+230);
    
    while(1)
    {
        display_swtich ^= 0x01;
        
        if(display_swtich)
        {
            smart_dma_g2rgb_run(cam0_buf, lcd_buf, 320*240);
            st7796_lcd_load(&s_lcd, (uint8_t*)lcd_buf, 0, 320, 0, 238);
        }
        else
        {
            smart_dma_g2rgb_run(cam1_buf, lcd_buf, 320*240);
            st7796_lcd_load(&s_lcd, (uint8_t*)lcd_buf, 0, 320, 240, 240+237);
        }

    }
}

int camera(void)
{
    int i, j;
    
    // SYSCON->AHBMATPRIO |= SYSCON_AHBMATPRIO_DMA1_MASK;
    rt_thread_t tid = rt_thread_find("tcam");
    
    if(tid == RT_NULL)
    {
        rt_thread_startup(rt_thread_create("tcam",    cam1_thread_entry, RT_NULL, 512, 21, 1));
        rt_thread_startup(rt_thread_create("tlcd", cam_lcd_thread_entry, RT_NULL, 512, 22, 1));
    }
    else
    {
        rt_kprintf("%s alread exist\r\n", tid->name);
    }

    return RT_EOK;
}



#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(camera, the dual camera demo);
#endif
