#include <os.h>
#include <rtthread.h>

#include <rthw.h>
#include <mmu.h>
#include <lwp_arch.h>
#include <cache.h>

#include <hal_atomic.h>
#include <hal_interrupt.h>
#include <ktimer.h>
#include <typedef.h>

int msleep(unsigned int msecs)
{
    rt_thread_mdelay(msecs);
    return 0;
}

unsigned long awos_arch_virt_to_phys(unsigned long virtaddr)
{
    return virtaddr - PV_OFFSET;
}

unsigned long awos_arch_phys_to_virt(unsigned long physaddr)
{
    return physaddr + PV_OFFSET;
}

#include <interrupt.h>
void enable_irq(unsigned int irq)
{
    rt_hw_interrupt_umask(irq);
}

void disable_irq(unsigned int irq)
{
    rt_hw_interrupt_mask(irq);
}

const void *free_irq(unsigned int irq, void *dev_id)
{
    const void *ret = RT_NULL;

    return ret;
}

int request_threaded_irq(unsigned int irq, irq_handler_t handler,
                         irq_handler_t thread_fn, unsigned long irqflags,
                         const char *devname, void *dev_id)
{
    rt_hw_interrupt_install(irq, (rt_isr_handler_t)handler, dev_id, devname);
    return 0;
}


void awos_arch_mems_flush_icache_region(unsigned long start, unsigned long len)
{
    rt_hw_cpu_icache_ops(RT_HW_CACHE_INVALIDATE, (void *)start, len);
}

void awos_arch_mems_clean_dcache_region(unsigned long start, unsigned long len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_FLUSH, (void *)start, len);
}

void awos_arch_mems_clean_flush_dcache_region(unsigned long start, unsigned long len)
{
    rt_hw_cpu_dcache_clean_flush((void *)start, len);
}

void awos_arch_mems_flush_dcache_region(unsigned long start, unsigned long len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)start, len);
}

void awos_arch_clean_flush_cache(void)
{
    rt_hw_cpu_dcache_clean_all();
    rt_hw_cpu_icache_invalidate_all();
}

void awos_arch_clean_flush_cache_region(unsigned long start, unsigned long len)
{
    rt_hw_cpu_dcache_ops(RT_HW_CACHE_INVALIDATE, (void *)start, len);
    rt_hw_cpu_icache_ops(RT_HW_CACHE_INVALIDATE, (void *)start, len);
}

void awos_arch_flush_cache(void)
{
    rt_hw_cpu_dcache_invalidate_all();
    rt_hw_cpu_icache_invalidate_all();
}

void awos_arch_clean_dcache(void)
{
    rt_hw_cpu_dcache_clean_all();
}

void awos_arch_clean_flush_dcache(void)
{
    rt_hw_cpu_dcache_clean_all();
    rt_hw_cpu_dcache_invalidate_all();
}

void awos_arch_flush_dcache(void)
{
    rt_hw_cpu_dcache_invalidate_all();
}

void awos_arch_flush_icache_all(void)
{
    rt_hw_cpu_icache_invalidate_all();
}

int32_t esCFG_GetGPIOSecKeyCount(char *GPIOSecName)
{
    int id;

    if (!rt_strcmp(GPIOSecName, "pwm2") || !rt_strcmp(GPIOSecName, "pwm7"))
    {
        return 1;
    }
    else if (!rt_strcmp(GPIOSecName, "sdc0") || !rt_strcmp(GPIOSecName, "sdc1"))
    {
        return 6;
    }
    else if (!rt_strcmp(GPIOSecName, "spi0"))
    {
        return 6;
    }
    else if (sscanf(GPIOSecName, "twi%d", &id) == 1)
    {
        return 2;
    }
    return 0;
}

RT_WEAK int hal_i2c_gpio_cfg_load(user_gpio_set_t *gpio_cfg, int32_t GPIONum, int id)
{
    rt_kprintf("FUNCTION:%s not implemented.\n", __FUNCTION__);
    return -1;
}

#define CFG_GPIO_PORT(p) ((p) - 'A' + 1)

int32_t esCFG_GetGPIOSecData(char *GPIOSecName, void *pGPIOCfg, int32_t GPIONum)
{
    user_gpio_set_t *gpio_cfg = (user_gpio_set_t *) pGPIOCfg;
    int i;
    int id;

    if (!rt_strcmp(GPIOSecName, "pwm2"))
    {
        rt_strncpy(gpio_cfg->gpio_name, "PD18", 5);
        gpio_cfg->data = 0;
        gpio_cfg->drv_level = 3;
        gpio_cfg->mul_sel = 5; // PWM
        gpio_cfg->port = CFG_GPIO_PORT('D'); // PORT-D
        gpio_cfg->port_num = 18; // PD18
        gpio_cfg->pull = 0; // pull disable
    }
    else if (!rt_strcmp(GPIOSecName, "pwm7"))
    {
        rt_strncpy(gpio_cfg->gpio_name, "PD22", 5);
        gpio_cfg->data = 0;
        gpio_cfg->drv_level = 3;
        gpio_cfg->mul_sel = 5; // PWM
        gpio_cfg->port = CFG_GPIO_PORT('D'); // PORT-D
        gpio_cfg->port_num = 22; // PD22
        gpio_cfg->pull = 0; // pull disable
    }
    else if (!rt_strcmp(GPIOSecName, "sdc0"))
    {
        /*
        [sdc0]
        ;sdc0_used          = 1
        ;bus-width      = 4
        sdc0_d1            = port:PF00<2><1><1><default>
        sdc0_d0            = port:PF01<2><1><1><default>
        sdc0_clk           = port:PF02<2><1><1><default>
        sdc0_cmd           = port:PF03<2><1><1><default>
        sdc0_d3            = port:PF04<2><1><1><default>
        sdc0_d2            = port:PF05<2><1><1><default>
        */
        for (i = 0; i < GPIONum; i++)
        {
            strcpy(gpio_cfg->gpio_name, GPIOSecName);
            gpio_cfg->port = CFG_GPIO_PORT('F');
            gpio_cfg->port_num = i;
            gpio_cfg->mul_sel = 2;
            gpio_cfg->pull = 1;
            gpio_cfg->drv_level = 1;
            gpio_cfg->data = 0;
            gpio_cfg++;
        }
    }
    else if (!rt_strcmp(GPIOSecName, "sdc1"))
    {
        /*
        [sdc1]
        ;sdc1_used          = 1
        ;bus-width= 4
        sdc1_clk           = port:PG00<2><1><1><default>
        sdc1_cmd           = port:PG01<2><1><1><default>
        sdc1_d0            = port:PG02<2><1><1><default>
        sdc1_d1            = port:PG03<2><1><1><default>
        sdc1_d2            = port:PG04<2><1><1><default>
        sdc1_d3            = port:PG05<2><1><1><default>
        */
        for (i = 0; i < GPIONum; i++)
        {
            strcpy(gpio_cfg->gpio_name, GPIOSecName);
            gpio_cfg->port = CFG_GPIO_PORT('G');
            gpio_cfg->port_num = i;
            gpio_cfg->mul_sel = 2;
            gpio_cfg->pull = 1;
            gpio_cfg->drv_level = 1;
            gpio_cfg->data = 0;
            gpio_cfg++;
        }
    }
    else if (!rt_strcmp(GPIOSecName, "spi0"))
    {
        /*
        ;----------------------------------------------------------------------------------
        ;SPI controller configuration
        ;----------------------------------------------------------------------------------
        [spi0]
        spi0_sclk           = port:PC02<2><0><2><default>
        spi0_cs             = port:PC03<2><1><2><default>
        spi0_mosi           = port:PC04<2><0><2><default>
        spi0_miso           = port:PC05<2><0><2><default>
        spi0_wp             = port:PC06<2><0><2><default>
        spi0_hold           = port:PC07<2><0><2><default>
        */
        for (i = 0; i < GPIONum; i++)
        {
            strcpy(gpio_cfg->gpio_name, GPIOSecName);
            gpio_cfg->port = CFG_GPIO_PORT('C');
            gpio_cfg->port_num = 2 + i;
            gpio_cfg->mul_sel = 2;
            gpio_cfg->pull = 0;
            gpio_cfg->drv_level = 2;
            gpio_cfg->data = 0;
            gpio_cfg++;
        }
        gpio_cfg = (user_gpio_set_t *) pGPIOCfg;
        gpio_cfg[1].pull = 1;
    }
    else if (sscanf(GPIOSecName, "twi%d", &id) == 1)
    {
        extern int hal_i2c_gpio_cfg_load(user_gpio_set_t *gpio_cfg, int32_t GPIONum, int id);
        return hal_i2c_gpio_cfg_load(gpio_cfg, GPIONum, id);
    }
    return 0;
}

int32_t esCFG_GetKeyValue(char *SecName, char *KeyName, int32_t Value[], int32_t Count)
{
    if (!rt_strcmp("target", SecName))
    {
        /*
        [target]
        boot_clock      = 1008
        storage_type    = 1
        */
        if (!rt_strcmp("storage_type", KeyName))
        {
            if (Count == 1)
            {
                *Value = 1;
                return 0;
            }
        }
        else if (!rt_strcmp("boot_clock", KeyName))
        {
            if (Count == 1)
            {
                *Value = 1008;
                return 0;
            }
        }
    }
    else if (!rt_strcmp("card_product", SecName))
    {
        /*
        [card_product]
        card_product_used    = 1
        card_product_storage = 3
        */
        if (!rt_strcmp("card_product_used", KeyName))
        {
            if (Count == 1)
            {
                *Value = 1;
                return 0;
            }
        } else if (!rt_strcmp("card_product_storage", KeyName))
        {
            if (Count == 1)
            {
                *Value = 3;
                return 0;
            }
        }
    }
    else if (!rt_strcmp("sdcard_global", SecName))
    {
        /*
        [sdcard_global]
        used_card_no    = 0x01
        ;used_card_no = 0x01, when use card0
        ;used_card_no = 0x02, when use card1
        ;used_card_no = 0x03, when use card0 & card1
        internal_card = 0x00
        ;internal_card = 0x00, 无内置卡内置卡
        ;internal_card = 0x01, card0 做内置卡
        ;internal_card = 0x02, card1 做内置卡
        */
        if (!rt_strcmp("internal_card", KeyName))
        {
            *Value = 0x00;
            return 0;
        } else if (!rt_strcmp("used_card_no", KeyName))
        {
            *Value = 0x01;
            return 0;
        }
    }
    return -1;
}

int do_gettimeofday(struct timespec64 *ts)
{
    if (ts)
    {
        ts->tv_sec = rt_tick_get() / RT_TICK_PER_SECOND;
        ts->tv_nsec = (rt_tick_get() % RT_TICK_PER_SECOND) * (1000000000 / RT_TICK_PER_SECOND);
    }
    return 0;
}
