#ifndef __DRV_LCD_FLEXIO_H
#define __DRV_LCD_FLEXIO_H

#include <rthw.h>
#include <rtdevice.h>
#include "fsl_flexio_mculcd.h"
#include "fsl_flexio_mculcd_edma.h"
#include "st7796.h"

#define LCD_W 480
#define LCD_H 320

#define LCD_FLEXIO_USING_EDMA  0
#define LCD_FLEXIO_USING_SPI   0

typedef struct
{
    struct rt_device            parent;
    FLEXIO_MCULCD_Type          mculcd;
    st7796_lcd_t                st7796;
    DMA_Type                    *DMAX;
    rt_sem_t                    sem;
    flexio_mculcd_edma_handle_t dma_hdl;
    edma_handle_t               tx_handle;
    struct rt_spi_device        *spi_dev;
} nxplcd_t;


#endif
