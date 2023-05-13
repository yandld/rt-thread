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


#define CAM_W       (160)
#define CAM_H       (120)

#define CAM_RST_PIN     (1*32+19)
#define ALPHA           (0.8)

static rt_device_t cam[2];

void smart_dma_g2rgb_init(void);
void smart_dma_g2rgb_run(void *input, void *output, uint32_t size);

//uint8_t cam_buf[CAM_W*CAM_H]         __attribute__ ((section(".ARM.__at_0x04000000")));
//uint8_t prv_cam_buf[CAM_W*CAM_H]     __attribute__ ((section(".ARM.__at_0x04005000")));
//uint8_t cam_buf_diff[CAM_W*CAM_H]    __attribute__ ((section(".ARM.__at_0x0400A000")));

uint8_t cam_buf[CAM_W*CAM_H];
uint8_t prv_cam_buf[CAM_W*CAM_H];
uint8_t cam_buf_diff[CAM_W*CAM_H];

uint16_t lcd_buf[CAM_W*CAM_H];
int sum = 0, last_sum=0, diff_sum=0;

uint32_t cam_start_xfer(rt_device_t dev, uint8_t *buf);
int rt_hw_hm0360_init(void);

static rt_sem_t cam_sem[2];
extern uint8_t cam_idx;
int frame_cnt = 0;

rt_err_t cam0_rx_indicate(rt_device_t dev, rt_size_t size)
{    
    rt_sem_release(cam_sem[0]);
    return RT_EOK;
}

rt_err_t cam1_rx_indicate(rt_device_t dev, rt_size_t size)
{    
    rt_sem_release(cam_sem[1]);
    return RT_EOK;
}


static void cam_thread_entry(void *parameter)
{
    int i, j;
    smart_dma_g2rgb_init();
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

    cam[0] = rt_device_find("cam0");
    if(cam[0])
    {
        cam_sem[0] = rt_sem_create("cam_sem", 0, RT_IPC_FLAG_FIFO);
        cam[0]->rx_indicate = cam0_rx_indicate;
        rt_device_open(cam[0], RT_DEVICE_OFLAG_RDWR);
    }
    
    cam[1] = rt_device_find("cam1");
    if(cam[1])
    {
        cam_sem[1] = rt_sem_create("cam_sem", 0, RT_IPC_FLAG_FIFO);
        cam[1]->rx_indicate = cam1_rx_indicate;
        rt_device_open(cam[1], RT_DEVICE_OFLAG_RDWR);
    }
   
    while(1)
    {
        if(cam_idx == 0)
        {
            cam[0]->rx_indicate = cam0_rx_indicate;
            cam[1]->rx_indicate = RT_NULL;
        }
        
        if(cam_idx == 1)
        {
            cam[0]->rx_indicate = RT_NULL;
            cam[1]->rx_indicate = cam1_rx_indicate;
        }
        
        if(rt_sem_take(cam_sem[cam_idx], RT_WAITING_FOREVER) == RT_EOK)
        {
            sum = 0;
            for(i=0; i<CAM_W*CAM_H; i++)
            {
                cam_buf_diff[i] = abs(cam_buf[i] - prv_cam_buf[i]);
                sum += cam_buf_diff[i];
            }

            rt_kprintf("idx:%d, sum:%d, last_sum:%d, frame_cnt:%d, diff_sum:%d\r\n", cam_idx, sum, last_sum, frame_cnt, diff_sum);

            diff_sum = diff_sum*(ALPHA) + (1-ALPHA)*abs(sum - last_sum);
            
            smart_dma_g2rgb_run(cam_buf, lcd_buf, CAM_W*CAM_H);
            memcpy(prv_cam_buf, cam_buf, CAM_W*CAM_H);
            last_sum = sum;
            cam_start_xfer(cam[cam_idx], cam_buf);
            frame_cnt++;
        }

    }
}



int eb_cam(void)
{
    int i, j;
    
    rt_thread_t tid = rt_thread_find("tcam");
    
    if(tid == RT_NULL)
    {
        rt_thread_startup(rt_thread_create("tcam",    cam_thread_entry, RT_NULL, 1024, 22, 1));
    }
    else
    {
        rt_kprintf("%s alread exist\r\n", tid->name);
    }

    return RT_EOK;
}



#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(eb_cam, the dual camera demo);
#endif
