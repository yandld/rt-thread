#include <rtdevice.h>
#include <rthw.h>
#include "fsl_common.h"

#define I2C0_SDA_PIN     (16)
#define I2C0_SCL_PIN     (17)

#define I2C1_SDA_PIN     (4*32 + 2)
#define I2C1_SCL_PIN     (4*32 + 3)


static void at32_set_sda(void *data, rt_int32_t state)
{
  //  rt_kprintf("at32_set_sda\r\n");
    rt_pin_write(I2C0_SDA_PIN, state);
}

static void at32_set_scl(void *data, rt_int32_t state)
{
   // rt_kprintf("at32_set_scl:%d\r\n", state);
    rt_pin_write(I2C0_SCL_PIN, state);
}

static rt_int32_t at32_get_sda(void *data)
{
   // rt_kprintf("at32_get_sda\r\n");
    return rt_pin_read(I2C0_SDA_PIN);
}

static rt_int32_t at32_get_scl(void *data)
{
    uint32_t val =  rt_pin_read(I2C0_SCL_PIN);
   // rt_kprintf("at32_get_scl:%d\r\n", val);
    return val;
}

static void _udelay(rt_uint32_t us)
{
    rt_hw_us_delay(us);
}


static const struct rt_i2c_bit_ops bit_ops = {
	RT_NULL,
	at32_set_sda,
	at32_set_scl,
	at32_get_sda,
	at32_get_scl,

	_udelay,

	10,
	20
};


int rt_i2c_init(void)
{
	struct rt_i2c_bus_device *bus;

	bus = rt_malloc(sizeof(struct rt_i2c_bus_device));

	rt_memset((void *)bus, 0, sizeof(struct rt_i2c_bus_device));

	bus->priv = (void *)&bit_ops;

	rt_i2c_bit_add_bus(bus, "i2c0");

    rt_pin_mode(I2C0_SCL_PIN, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(I2C0_SDA_PIN, PIN_MODE_OUTPUT_OD);
    rt_pin_write(I2C0_SCL_PIN, 1);
    rt_pin_write(I2C0_SDA_PIN, 1);
    

	return RT_EOK;
}

INIT_DEVICE_EXPORT(rt_i2c_init);

