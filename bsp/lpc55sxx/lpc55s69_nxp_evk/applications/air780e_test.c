
#include <rtthread.h>



static rt_err_t serial_rx_ind(rt_device_t dev, rt_size_t size)
{
    static uint8_t buf[64];

    int len;
    len = rt_device_read(dev, 0, buf, len);
    
    rt_kprintf("recv:%d\r\n", len);

    return RT_EOK;
}


static void air780e_test(int argc, char **argv)
{
    rt_device_t dev = rt_device_find("uart7");
    rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    
    rt_device_write(dev, 0, "ABCDEDF", 8);
    rt_device_set_rx_indicate(dev, serial_rx_ind);
}
MSH_CMD_EXPORT(air780e_test, air780e test);
