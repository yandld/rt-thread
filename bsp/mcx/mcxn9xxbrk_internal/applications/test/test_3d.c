#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "3DTest.h"



static int switch_screen = 0;


static void irq_tmr(void *parameter)
{ 
    switch_screen++;
    switch_screen %= 5;
}

int test_custom(void)
{
    int i;
    
    rt_timer_start(rt_timer_create("mtmr", irq_tmr, RT_NULL, rt_tick_from_millisecond(2000), RT_TIMER_FLAG_PERIODIC));
    while(1)
    {
        render3D(switch_screen);
        rt_thread_mdelay(1);
    }
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(test_custom, test_custom);
#endif

//INIT_APP_EXPORT(test_custom);

