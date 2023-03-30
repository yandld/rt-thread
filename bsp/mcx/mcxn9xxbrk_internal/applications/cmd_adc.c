#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>


#include "stdlib.h"
#include "fsl_device_registers.h"
#include "fsl_lpadc.h"
#include "pin_mux.h"

#include "clock_config.h"
#include "fsl_lpadc.h"
#include "fsl_common.h"
#include "fsl_ctimer.h"
#include "basic_statistics.h"




#define TEST_SIZE               (400)
#define ADC_CHL                 (0)
#define DEMO_LPADC_BASE         ADC0
#define DEMO_LPADC_USER_CMDID   1U


#define CTIMER          CTIMER4         /* Timer 4 */


static uint32_t last_tick;

static void irq_tmr(void *parameter)
{
    rt_kprintf("%d\r\n", CTIMER->TC - last_tick);
    last_tick = CTIMER->TC;
}

int adc(void)
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
    
    lpadc_config_t adc_config;
    lpadc_conv_trigger_config_t trig_config;
    lpadc_conv_command_config_t cmd_cfg;
    lpadc_conv_result_t mLpadcResultConfigStruct;

    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 2);
    CLOCK_AttachClk(kCLK_IN_to_ADC0);

    /* enable VREF */
    SPC0->ACTIVE_CFG1 |= 0x1;


    rt_kprintf("ADC0 CLOCK:%d Hz\r\n", CLOCK_GetAdcClkFreq(0));
    rt_kprintf("ADC1 CLOCK:%d Hz\r\n", CLOCK_GetAdcClkFreq(1));
    
    LPADC_GetDefaultConfig(&adc_config);
    adc_config.enableAnalogPreliminary = true;
/*
CFG[REFSEL]=00, VREFH reference pin
CFG[REFSEL]=01, ANA_7(VREFI/VREFO) pin
CFG[REFSEL]=10, VDDA supply pin
*/
    adc_config.referenceVoltageSource = 0;
    adc_config.conversionAverageMode = kLPADC_ConversionAverage128;
    adc_config.powerLevelMode = kLPADC_PowerLevelAlt4;
    adc_config.enableConvPause       = false;
    adc_config.convPauseDelay        = 0;
    
    LPADC_Init(DEMO_LPADC_BASE, &adc_config);
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
        
    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&cmd_cfg);
    cmd_cfg.channelNumber = ADC_CHL;
    cmd_cfg.channelBNumber = ADC_CHL;
    cmd_cfg.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
    cmd_cfg.hardwareAverageMode = kLPADC_HardwareAverageCount4;
    cmd_cfg.loopCount = 0;
    cmd_cfg.sampleTimeMode = kLPADC_SampleTimeADCK7;
    cmd_cfg.enableChannelB = true;
/*
    kLPADC_SampleChannelSingleEndSideA = 0U,
    kLPADC_SampleChannelSingleEndSideB = 1U, 
    kLPADC_SampleChannelDiffBothSide = 2U, 
    kLPADC_SampleChannelDualSingleEndBothSide =
*/
    cmd_cfg.sampleChannelMode = kLPADC_SampleChannelSingleEndSideA;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &cmd_cfg);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&trig_config);
    trig_config.targetCommandId       = DEMO_LPADC_USER_CMDID;
    trig_config.enableHardwareTrigger = false;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &trig_config); /* Configurate the trigger0. */

    rt_kprintf("ADC_CFG:0x%08X\r\n", ADC0->CFG);
    rt_kprintf("ADC_CTL:0x%08X\r\n", ADC0->CTRL);
    
    int32_t tmp32;
    uint16_t data;
    
    bs_t bs;
    bs_init(&bs, TEST_SIZE);
    
    for(i=0; i<TEST_SIZE; i++)
    {
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 0))
        {
        }
    }
    
    tick = CTIMER->TC;
    for(i=0; i<TEST_SIZE; i++)
    {
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 0))
        {
        }
        bs_add_sample(&bs, mLpadcResultConfigStruct.convValue);
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
MSH_CMD_EXPORT(adc, the adc adc adc);
#endif
