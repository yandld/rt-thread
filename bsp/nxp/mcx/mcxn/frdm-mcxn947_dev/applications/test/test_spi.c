#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#define GPIO_CS     (12)


rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, rt_uint32_t pin);

int test_spi(void)
{
    int i;
    rt_hw_spi_device_attach("spi3", "spi31", GPIO_CS);
    
    rt_kprintf("test_spi\r\n");

        rt_thread_mdelay(100);
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(test_spi, test_spi);
#endif

