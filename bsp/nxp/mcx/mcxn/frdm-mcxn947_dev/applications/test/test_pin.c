#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#define BUTTON_PIN1       ((0*32)+23)
#define BUTTON_PIN2      ((0*32)+6)
#define LED_PIN          ((0*32)+10)

static void pin_irq1(void *args)
{
    rt_kprintf("BUTTON_PIN1 cb\r\n");
}


static void pin_irq2(void *args)
{
    rt_kprintf("BUTTON_PIN2 cb\r\n");
}


static void irq_tmr(void *parameter)
{ 
    static uint8_t toggle = 0;
    rt_pin_write(LED_PIN, toggle);
    toggle ^= 0x01;
}

int test_pin(void)
{
    rt_pin_mode(BUTTON_PIN1, PIN_MODE_INPUT_PULLUP); 
    rt_pin_attach_irq(BUTTON_PIN1, PIN_IRQ_MODE_FALLING, pin_irq1, RT_NULL);
    rt_pin_irq_enable(BUTTON_PIN1, 1);
    
    rt_pin_mode(BUTTON_PIN2, PIN_MODE_INPUT_PULLUP); 
    rt_pin_attach_irq(BUTTON_PIN2, PIN_IRQ_MODE_FALLING, pin_irq2, RT_NULL);
    rt_pin_irq_enable(BUTTON_PIN2, 1);
    
    rt_pin_mode(LED_PIN, PIN_MODE_OUTPUT); 
    rt_timer_start(rt_timer_create("mtmr", irq_tmr, RT_NULL, rt_tick_from_millisecond(500), RT_TIMER_FLAG_PERIODIC));
    return RT_EOK;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(test_pin, test_pin);
#endif

INIT_APP_EXPORT(test_pin);
