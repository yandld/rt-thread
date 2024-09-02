#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "3DTest.h"

static int switch_screen = 0;
static rt_sem_t sem;

static void irq_tmr(void *parameter)
{ 
    switch_screen++;
    switch_screen %= 500;
    
    rt_sem_release(sem);
}


static void thread_3d_entry(void *parameter)
{
    while(1)
    {
        rt_sem_take(sem, RT_WAITING_FOREVER);
        render3D(switch_screen);
    }
}

int test_3d(void)
{
    sem = rt_sem_create("sem", 0, RT_IPC_FLAG_FIFO);
    rt_timer_start(rt_timer_create("mtmr", irq_tmr, RT_NULL, rt_tick_from_millisecond(10), RT_TIMER_FLAG_PERIODIC));
    rt_thread_startup(rt_thread_create("t3d", thread_3d_entry, RT_NULL, 1024, 22, 1));
    return RT_EOK;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(test_3d, test_3d);
#endif

//INIT_APP_EXPORT(test_3d);



