
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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





#define DEMO_LPSPI_EDMA_WATERMARK   4U
#define DEMO_LPSPI_EDMA_MAJOR_LINES 240U /* Must be integer counts! */

#define DEMO_LPSPI_RES_HORIZONTAL   320UL
#define DEMO_LPSPI_RES_VERTICAL     240UL
#define DEMO_LPSPI_PIX_SIZE         1U
#define DEMO_LPSPI_LINE_SIZE        (DEMO_LPSPI_RES_HORIZONTAL * DEMO_LPSPI_PIX_SIZE)
#define DEMO_LPSPI_BUFFER_SIZE      (DEMO_LPSPI_LINE_SIZE * DEMO_LPSPI_RES_VERTICAL)

#define VSYNC_PIN      ((0*32)+15)

typedef struct
{
    LPSPI_Type              *LPSPIX;
    DMA_Type                *DMAX;
    uint32_t                rx_dma_ch;
    edma_handle_t           dma_rx_handle;
    uint8_t                 buf[2][DEMO_LPSPI_BUFFER_SIZE] __attribute__ ((aligned (4)));
    dma_request_source_t    dma_req_src;
    uint8_t                 swtich_flag;
    uint8_t                 vsync_flag;
}lpspi_quad_camera_t;


void hm0360_Init(void);
static status_t lpfc_quad_init(lpspi_quad_camera_t *camera);
static status_t lpfc_quad_test_xfer(lpspi_quad_camera_t *camera, uint8_t *buf);


lpspi_quad_camera_t cam = 
{
    .LPSPIX = LPSPI3,
    .DMAX = DMA0,
    .rx_dma_ch = 2,
    .dma_req_src = kDmaRequestMuxLpFlexcomm3Rx,
    .swtich_flag = 0,
    .vsync_flag  = 0,
};


uint16_t  lcd_buf[DEMO_LPSPI_RES_HORIZONTAL*DEMO_LPSPI_RES_VERTICAL] __attribute__ ((aligned (32)));



static void gray2rgb565(uint8_t *gray_buf, uint16_t* rgb565_buf, uint32_t len)
{
    int i;
    for(i=0; i<len; i++)
    {
     //   uint8_t byte = (gray_buf[i]>> 4) | ((gray_buf[i] & 0x0F)<<4);
     //   gray_buf[i] = byte;
        rgb565_buf[i] = ((gray_buf[i] >> 3) << 11) | ((gray_buf[i] >> 2) << 5) | (gray_buf[i] >> 3);
    }
    
}


static void lpfc_edma_major_callback(edma_handle_t *handle, void *userData, bool transferDone, uint32_t tcds)
{
    rt_kprintf("dma_done\r\n");
    DMA0->CH[cam.rx_dma_ch].TCD_CSR |= DMA_TCD_CSR_DREQ_MASK;
}

static status_t lpfc_quad_init(lpspi_quad_camera_t *camera)
{

    lpspi_slave_config_t slave_cfg;
    LPSPI_SlaveGetDefaultConfig(&slave_cfg);

    slave_cfg.bitsPerFrame       = 8U;
    slave_cfg.whichPcs           = kLPSPI_Pcs0;
    slave_cfg.dataOutConfig      = kLpspiDataOutTristate;
    slave_cfg.pcsFunc            = kLPSPI_PcsAsData;
    slave_cfg.pcsActiveHighOrLow = kLPSPI_PcsActiveHigh;
//    slave_cfg.cpol         = kLPSPI_ClockPolarityActiveLow;
    slave_cfg.cpha         = kLPSPI_ClockPhaseSecondEdge;

    LPSPI_SlaveInit(camera->LPSPIX, &slave_cfg);

    LPSPI_Enable(camera->LPSPIX, false);

    /* !! watermark must be exact NBYTES, DMA request will be asserted when data count is ABOVE WM */
    LPSPI_SetFifoWatermarks(camera->LPSPIX, 0, DEMO_LPSPI_EDMA_WATERMARK - 1);
    LPSPI_FlushFifo(camera->LPSPIX, false, true);

    uint32_t tcr = camera->LPSPIX->TCR;

    tcr &= ~(LPSPI_TCR_CONT_MASK | LPSPI_TCR_CONTC_MASK | LPSPI_TCR_BYSW_MASK | LPSPI_TCR_TXMSK_MASK);
    tcr |= LPSPI_TCR_TXMSK(1);
    tcr |= LPSPI_TCR_BYSW(0);
    tcr |= LPSPI_TCR_PCS(0);
    tcr |= LPSPI_TCR_WIDTH(2);

    LPSPI_Enable(camera->LPSPIX, true);

    camera->LPSPIX->TCR = tcr;



    /* CHn_MUX can not be set again if the specified request is in use. */
    /* Also, the existing requests should be cleared before setting to another channel */
    EDMA_SetChannelMux(DMA0, camera->rx_dma_ch, camera->dma_req_src);
    EDMA_CreateHandle(&camera->dma_rx_handle, DMA0, camera->rx_dma_ch);
    EDMA_SetCallback(&camera->dma_rx_handle, lpfc_edma_major_callback, NULL);

    return kStatus_Success;
}

static status_t lpfc_quad_test_xfer(lpspi_quad_camera_t *camera, uint8_t *buf)
{
    status_t ret = kStatus_Success;

    /**
     * DMA configuration:
     * Each DMA request will read 4 bytes from LPSPI FIFO
     * Each major loop will copy frame to memory
     * Major loop interrup is not necessary if actual VSYNC is used.
     * DMA maximum supports 131072 (32768 * 4) bytes in a single major loop,
     * this code has no byte count limit by modifying the macro.
     */

    edma_transfer_config_t dma_cfg;

    memset(&dma_cfg, 0x00, sizeof(dma_cfg));

    dma_cfg.srcAddr            = LPSPI_GetRxRegisterAddress(camera->LPSPIX);
    dma_cfg.srcTransferSize    = kEDMA_TransferSize1Bytes;
    dma_cfg.srcOffset          = 0;
    dma_cfg.destAddr           = (uint32_t)buf;
    dma_cfg.destTransferSize   = kEDMA_TransferSize4Bytes;
    dma_cfg.destOffset         = 4;
    dma_cfg.minorLoopBytes     = DEMO_LPSPI_EDMA_WATERMARK; /* NBYTES for each DMA request */
    dma_cfg.majorLoopCounts    = (DEMO_LPSPI_LINE_SIZE * DEMO_LPSPI_EDMA_MAJOR_LINES) / DEMO_LPSPI_EDMA_WATERMARK;
    dma_cfg.dstMajorLoopOffset = 0; /* !! DADDR is auto-incremented by hardware, no adjustment needed */

    EDMA_SetTransferConfig(DMA0, camera->rx_dma_ch, &dma_cfg, NULL);
    EDMA_EnableChannelInterrupts(DMA0, camera->rx_dma_ch, kEDMA_MajorInterruptEnable);

    uint32_t tcd_xfer_count = DEMO_LPSPI_RES_VERTICAL / DEMO_LPSPI_EDMA_MAJOR_LINES;

    if (tcd_xfer_count > 1) {
        /* Do not clear request after TCD completed, since we need this channel to be triggered again by request */
        DMA0->CH[camera->rx_dma_ch].TCD_CSR &= ~(DMA_TCD_CSR_DREQ_MASK);
    }

    /* Start DMA transfer */
    EDMA_StartTransfer(&camera->dma_rx_handle);

    /* Enable LPSPI DMA request */
    LPSPI_EnableDMA(camera->LPSPIX, kLPSPI_RxDmaEnable);

    return ret;
}



void vsync_irq(void *args)
{
    cam.vsync_flag = 1;
}


int camera(void)
{
    int i;

    /* SPI */
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 3U); /* 150MHz / 3 = 50MHz */
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM3);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 1U);
    
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 3U); /* 150MHz / 3 = 50MHz */
    CLOCK_AttachClk(kPLL_DIV_to_FLEXCOMM4);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1U);
    

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

    
    hm0360_Init();
    
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

    lpfc_quad_init(&cam);
    
    /* VSYNC */
    rt_pin_mode(VSYNC_PIN, PIN_MODE_INPUT); 
    rt_pin_attach_irq(VSYNC_PIN, PIN_IRQ_MODE_FALLING, vsync_irq, RT_NULL);
    rt_pin_irq_enable(VSYNC_PIN, 1);
    
    lpfc_quad_test_xfer(&cam, &cam.buf[cam.swtich_flag][0]);
    while(1)
    {
        if(cam.vsync_flag)
        {
            cam.vsync_flag = 0;
//            LPSPI_DisableDMA(cam.LPSPIX, kLPSPI_RxDmaEnable);
//            EDMA_AbortTransfer(&cam.dma_rx_handle);
            cam.swtich_flag ^= 0x01;
            lpfc_quad_test_xfer(&cam, &cam.buf[cam.swtich_flag][0]);
            
            gray2rgb565(&cam.buf[!cam.swtich_flag][0], lcd_buf, 320*240);

            AreaPoints_t area;
            
            area.x1 = 0;
            area.x2 = 320;
            area.y1 = 240;
            area.y2 = 240+240;
            
            LCD_SetWindow(&area);

            FLEXIO_8080_MulBeatWR_nPrm(0x2C, lcd_buf, 320*240);
//    while(WR_DMATransferDone == false)
//    {
//        /* You can add some necessary statements here.*/
//    }
    
            rt_kprintf("VSYNC\r\n");
        }

        rt_thread_mdelay(1);
    }

}





#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(camera, dump memory trace for heap);
#endif
