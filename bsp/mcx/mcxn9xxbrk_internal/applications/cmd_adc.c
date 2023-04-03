#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#include "stdlib.h"
#include "fsl_device_registers.h"
#include "fsl_lpadc.h"
#include "pin_mux.h"

#include "clock_config.h"
#include "fsl_lpadc.h"
#include "fsl_spc.h"
#include "fsl_common.h"
#include "fsl_ctimer.h"
#include "basic_statistics.h"



/* 
ADC CHANNEL: 
27:  PMC BG+
28:  VREF BG+
29:  VBAT/4
*/

#define TEST_SIZE               (400)
#define ADC_CHL                 (0)
#define CTIMER          CTIMER4         /* Timer 4 */


static uint32_t last_tick;

static void irq_tmr(void *parameter)
{
    rt_kprintf("%d\r\n", CTIMER->TC - last_tick);
    last_tick = CTIMER->TC;
}

int adc_performance_test(void)
{
    int i;
    uint32_t tick;

    //SPC0->CNTRL &= ~(1<<2);
    rt_kprintf("SPC0->CNTRL:0x%08X\r\n", SPC0->CNTRL);
    
    PORT4->PCR[0]   = PORT_PCR_MUX(0) | PORT_PCR_PS(0) | PORT_PCR_PE(0) | PORT_PCR_IBE(1);     /* ANA_0, ADC0_A0 */
    PORT4->PCR[1]   = PORT_PCR_MUX(0) | PORT_PCR_PS(0) | PORT_PCR_PE(0) | PORT_PCR_IBE(1);     /* ANA_1, ADC0_B0 */
    
    ctimer_config_t config;
    ctimer_match_config_t matchConfig;
    CLOCK_SetClkDiv(kCLOCK_DivCtimer4Clk, CLOCK_GetCoreSysClkFreq() / (1000*1000) - 1);
    CLOCK_AttachClk(kPLL0_to_CTIMER4);
    
    CTIMER_GetDefaultConfig(&config);
    CTIMER_Init(CTIMER, &config);
    CTIMER_StartTimer(CTIMER);
    

    rt_adc_device_t adc_dev = (rt_adc_device_t)rt_device_find("adc0");
    
    RT_ASSERT(adc_dev != RT_NULL);
    
    rt_adc_enable(adc_dev, ADC_CHL);
    
    rt_kprintf("ADC0 CLOCK:%d Hz\r\n", CLOCK_GetAdcClkFreq(0));
    rt_kprintf("ADC1 CLOCK:%d Hz\r\n", CLOCK_GetAdcClkFreq(1));
    
    rt_kprintf("ADC_CFG:0x%08X\r\n", ADC0->CFG);
    rt_kprintf("ADC_CTL:0x%08X\r\n", ADC0->CTRL);
    

    uint32_t result = 0;
    
    bs_t bs;
    bs_init(&bs, TEST_SIZE);
    
    for(i=0; i<TEST_SIZE; i++)
    {
        result = rt_adc_read(adc_dev, ADC_CHL);
    }
    
    tick = CTIMER->TC;
    for(i=0; i<TEST_SIZE; i++)
    {
        result = rt_adc_read(adc_dev, ADC_CHL);
        bs_add_sample(&bs, result);
    }
    tick = CTIMER->TC - tick;
    
    bs_calc_result(&bs);
    double dus = tick;
    
    rt_kprintf("tick:%d, %fus, %fM\r\n", tick, dus, bs.cnt / dus);
        

    bs_dump(&bs, 0);
    bs_free(&bs);
    
    
    return RT_EOK;
}





#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(adc_performance_test, the adc adc adc);
#endif
