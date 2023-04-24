#ifndef __DRV_LCD_FLEXIO_H
#define __DRV_LCD_FLEXIO_H

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "fsl_lpadc.h"
#include "fsl_clock.h"
#include "fsl_flexio_mculcd.h"
#include "fsl_flexio_mculcd_edma.h"
#include "fsl_gpio.h"
#include "st7796.h"
#include "stdlib.h"

typedef struct
{
    struct rt_device    parent;
    FLEXIO_MCULCD_Type  mculcd;
    st7796_lcd_t        st7796;
} lcd_flexio_t;


int drv_lcd_flexio_hw_init(void);

#endif
