#include "drv_nxplcd_capt.h"

#define CAPT_I2C_DEV_NAME       "i2c2"
#define CAPT_I2C_ADDR           (0x5D)

static capt_t capt_obj;

static gt911_ret_t ctp_impl_xfer(void *handle, gt911_i2c_xfer_t *xfer)
{
    capt_t *capt = (capt_t*)handle;
    
    if(xfer->tx_len) rt_i2c_master_send(capt->bus, CAPT_I2C_ADDR, 0, xfer->tx_data, xfer->tx_len);
    if(xfer->rx_len) rt_i2c_master_recv(capt->bus, CAPT_I2C_ADDR, 0, xfer->rx_data, xfer->rx_len);

    return GT911_SUCCESS;
}
    
int drv_capt_hw_init(void)
{
    capt_obj.bus = (struct rt_i2c_bus_device*)rt_device_find(CAPT_I2C_DEV_NAME);
    
    if(capt_obj.bus == RT_NULL)
    {
        rt_kprintf("no %s device\r\n", CAPT_I2C_DEV_NAME);
        return -RT_ERROR;
    }
    
    capt_obj.gt911.user_data = capt_obj.parent.user_data = &capt_obj;
    capt_obj.gt911.ops.xfer = ctp_impl_xfer;
    
    if(gt911_ctp_init(&capt_obj.gt911) != GT911_SUCCESS)
    {
        return -RT_ERROR;
    }
    
    rt_device_register(&capt_obj.parent, "capt", RT_DEVICE_FLAG_RDWR);
    return RT_EOK;
}

INIT_DEVICE_EXPORT(drv_capt_hw_init);



static int capt_test(void)
{
    int i;
    gt911_input_t ctp_input;
    
    rt_device_t dev = rt_device_find("capt");
    
    RT_ASSERT(dev != RT_NULL);
    
    capt_t *capt = (capt_t*)dev->user_data;
    while(1)
    {
        gt911_ctp_read(&capt->gt911, &ctp_input);
        
        for (uint8_t i = 0; i < ctp_input.num_pos; i++) 
        {
            /* Found track ID #0 */
            if (ctp_input.pos[i].id == 0)
            {
                rt_kprintf("x:%d, y:%d\r\n", capt->gt911.pos_y_max - ctp_input.pos[i].pos_y, ctp_input.pos[i].pos_x);
            }
        }
    
        rt_thread_mdelay(16);
    }

    return RT_EOK;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(capt_test, the capt_test);
#endif


