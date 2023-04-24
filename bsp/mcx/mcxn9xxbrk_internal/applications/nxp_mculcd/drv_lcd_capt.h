#ifndef __DRV_LCD_FLEXIO_H
#define __DRV_LCD_FLEXIO_H

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "gt911_ctp.h"
#include "stdlib.h"

typedef struct
{
    struct rt_device            parent;
    struct rt_i2c_bus_device   *bus;
    gt911_t                     gt911;
} capt_t;




#endif
