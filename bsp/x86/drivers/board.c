/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-07-14     JasonHu      first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <rtconfig.h>
#include <rtdbg.h>

#include <cpuport.h>
#include <board.h>
#include <dma.h>

#include "drv_uart.h"
#include "direct_uart.h"
#include "drv_timer.h"
#include "pci.h"

#ifdef RT_USING_USERSPACE
#include <mmu.h>
#include <page.h>
#include "lwp_arch.h"

rt_mmu_info mmu_info;

/* kernel mmu table */
volatile rt_size_t g_mmu_table[ARCH_PAGE_SIZE / sizeof(rt_size_t)] __attribute__((aligned(ARCH_PAGE_SIZE)));

static size_t page_region_init()
{
    unsigned int memory_size = *((unsigned int *)0x000001000);
    rt_kprintf("physic memory size: %x bytes, %d MB\n", memory_size, memory_size / (1024 * 1024));
    if (memory_size < HW_PHY_MEM_SIZE_MIN)
    {
        dbg_log(DBG_ERROR, "phyisc memory too small! only %d MB, must >= %d MB\n",
                memory_size / (1024 * 1024), HW_PHY_MEM_SIZE_MIN / (1024 * 1024));
        for (;;)
        {
        }
    }
    if (memory_size > HW_PAGE_SIZE_MAX)
    {
        memory_size = HW_PAGE_SIZE_MAX;
    }
    return memory_size;
}

#endif

void rt_hw_board_init(void)
{
#ifdef BSP_USING_DIRECT_UART
    /* init direct serial hardware */
    rt_hw_direct_uart_init();
#endif /* RT_USING_DIRECT_UART */

#ifdef RT_USING_USERSPACE
    /* init page and mmu */
    rt_region_t init_page_region;
    init_page_region.start = (size_t)HW_PAGE_START;
    init_page_region.end = page_region_init();
    /* init no mapped area in kernel table, must in kernel space */
    RT_ASSERT(!rt_hw_mmu_map_init(&mmu_info, (void *)HW_KERNEL_DELAY_MAP_START,
              HW_KERNEL_DELAY_MAP_SIZE, (rt_size_t *)g_mmu_table, 0));

    rt_page_init(init_page_region);
    /* map kernel space, then can read/write this area directly. */
    rt_hw_mmu_kernel_map_init(&mmu_info, HW_KERNEL_START, HW_KERNEL_END);
    rt_hw_mmu_switch((void *)g_mmu_table);
    mmu_enable();
#endif

    /* init cpu special */
    rt_hw_cpu_init();

    /* initalize interrupt */
    rt_hw_interrupt_init();

#ifdef BSP_DRV_UART
    /* init serial driver */
    rt_hw_uart_init();
#endif  /* BSP_DRV_UART */

    /* init timer driver */
    rt_hw_timer_init();

#ifdef RT_USING_HEAP
    rt_kprintf("heap: [0x%08x - 0x%08x]\n", (rt_ubase_t) HW_HEAP_BEGIN, (rt_ubase_t) HW_HEAP_END);
    /* initialize memory system */
    rt_system_heap_init((void *)HW_HEAP_BEGIN, (void *)HW_HEAP_END);
#endif

    /* init dma allocator */
    rt_hw_dma_init(HW_DMA_BEGIN, HW_DMA_BEGIN + HW_DMA_SIZE);

    /* init pci bus */
    rt_pci_init();

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifdef RT_USING_CONSOLE
    /* set console device */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_CONSOLE */

}

void primary_cpu_entry(void)
{
    extern void entry(void);
    rt_hw_interrupt_disable();
    entry();
}
