#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <math.h>
#include "fsl_inputmux.h"
#include "fsl_freqme.h"
#include "stdio.h"
#include "fsl_lpadc.h"

#include "fsl_common.h"
#include "fsl_vref.h"

#ifndef ABS
#define ABS(a)         (((a) < 0) ? (-(a)) : (a))
#endif

#define DEMO_LPADC_BASE                  ADC0
#define DEMO_LPADC_IRQn                  ADC0_IRQn
#define DEMO_LPADC_IRQ_HANDLER_FUNC      ADC0_IRQHandler
#define DEMO_LPADC_TEMP_SENS_CHANNEL     26U
#define DEMO_LPADC_USER_CMDID            1U /* CMD1 */
#define DEMO_LPADC_SAMPLE_CHANNEL_MODE   kLPADC_SampleChannelDiffBothSide
#define DEMO_LPADC_VREF_SOURCE           kLPADC_ReferenceVoltageAlt2
#define DEMO_LPADC_DO_OFFSET_CALIBRATION false
#define DEMO_LPADC_OFFSET_VALUE_A        0x10U
#define DEMO_LPADC_OFFSET_VALUE_B        0x10U
#define DEMO_LPADC_USE_HIGH_RESOLUTION   true
#define DEMO_LPADC_TEMP_PARAMETER_A      FSL_FEATURE_LPADC_TEMP_PARAMETER_A
#define DEMO_LPADC_TEMP_PARAMETER_B      FSL_FEATURE_LPADC_TEMP_PARAMETER_B
#define DEMO_LPADC_TEMP_PARAMETER_ALPHA  FSL_FEATURE_LPADC_TEMP_PARAMETER_ALPHA
#define DEMO_VREF_BASE                   VREF0

lpadc_conv_command_config_t g_LpadcCommandConfigStruct; /* Structure to configure conversion command. */
volatile bool g_LpadcConversionCompletedFlag = false;
float g_CurrentTemperature                   = 0.0f;

const uint32_t g_LpadcFullRange = 65536U;


float DEMO_MeasureTemperature(ADC_Type *base, uint32_t commandId, uint32_t index)
{
    lpadc_conv_result_t convResultStruct;
    uint16_t Vbe1            = 0U;
    uint16_t Vbe8            = 0U;
    uint32_t convResultShift = 3U;
    float parameterSlope     = DEMO_LPADC_TEMP_PARAMETER_A;
    float parameterOffset    = DEMO_LPADC_TEMP_PARAMETER_B;
    float parameterAlpha     = DEMO_LPADC_TEMP_PARAMETER_ALPHA;
    float temperature        = -273.15f; /* Absolute zero degree as the incorrect return value. */

#if defined(FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE) && (FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE == 4U)
    /* For best temperature measure performance, the recommended LOOP Count should be 4, but the first two results is
     * useless. */
    /* Drop the useless result. */
    (void)LPADC_GetConvResult(base, &convResultStruct, (uint8_t)index);
    (void)LPADC_GetConvResult(base, &convResultStruct, (uint8_t)index);
#endif /* FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE */

    /* Read the 2 temperature sensor result. */
    if (true == LPADC_GetConvResult(base, &convResultStruct, (uint8_t)index))
    {
        Vbe1 = convResultStruct.convValue >> convResultShift;
        if (true == LPADC_GetConvResult(base, &convResultStruct, (uint8_t)index))
        {
            Vbe8 = convResultStruct.convValue >> convResultShift;
            /* Final temperature = A*[alpha*(Vbe8-Vbe1)/(Vbe8 + alpha*(Vbe8-Vbe1))] - B. */
            temperature = parameterSlope * (parameterAlpha * ((float)Vbe8 - (float)Vbe1) /
                                            ((float)Vbe8 + parameterAlpha * ((float)Vbe8 - (float)Vbe1))) -
                          parameterOffset;
        }
    }

    return temperature;
}


static void ADC_Configuration(void)
{
    lpadc_config_t lpadcConfigStruct;
    lpadc_conv_trigger_config_t lpadcTriggerConfigStruct;

    /* Init ADC peripheral. */
    LPADC_GetDefaultConfig(&lpadcConfigStruct);
    lpadcConfigStruct.enableAnalogPreliminary = true;
    lpadcConfigStruct.powerLevelMode          = kLPADC_PowerLevelAlt4;
#if defined(DEMO_LPADC_VREF_SOURCE)
    lpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
    lpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
#if defined(FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE)
    lpadcConfigStruct.FIFO0Watermark = FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE - 1U;
#endif /* FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE */
    LPADC_Init(DEMO_LPADC_BASE, &lpadcConfigStruct);
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_DoResetFIFO0(DEMO_LPADC_BASE);
#else
    LPADC_DoResetFIFO(DEMO_LPADC_BASE);
#endif

    /* Do ADC calibration. */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
    /* Request offset calibration. */
#if defined(DEMO_LPADC_DO_OFFSET_CALIBRATION) && DEMO_LPADC_DO_OFFSET_CALIBRATION
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
#else
    LPADC_SetOffsetValue(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_VALUE_A, DEMO_LPADC_OFFSET_VALUE_B);
#endif /* DEMO_LPADC_DO_OFFSET_CALIBRATION */
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */
    /* Request gain calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&g_LpadcCommandConfigStruct);
    g_LpadcCommandConfigStruct.channelNumber       = DEMO_LPADC_TEMP_SENS_CHANNEL;
    g_LpadcCommandConfigStruct.sampleChannelMode   = DEMO_LPADC_SAMPLE_CHANNEL_MODE;
    g_LpadcCommandConfigStruct.sampleTimeMode      = kLPADC_SampleTimeADCK131;
    g_LpadcCommandConfigStruct.hardwareAverageMode = kLPADC_HardwareAverageCount128;
#if defined(FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE)
    g_LpadcCommandConfigStruct.loopCount = FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE - 1U;
#endif /* FSL_FEATURE_LPADC_TEMP_SENS_BUFFER_SIZE */
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE
    g_LpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* FSL_FEATURE_LPADC_HAS_CMDL_MODE */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &g_LpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&lpadcTriggerConfigStruct);
    lpadcTriggerConfigStruct.targetCommandId = DEMO_LPADC_USER_CMDID;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &lpadcTriggerConfigStruct); /* Configurate the trigger0. */

    /* Enable the watermark interrupt. */
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFO0WatermarkInterruptEnable);
#else
    LPADC_EnableInterrupts(DEMO_LPADC_BASE, kLPADC_FIFOWatermarkInterruptEnable);
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
    EnableIRQ(DEMO_LPADC_IRQn);

    /* Eliminate the first two inaccurate results. */
    LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
    while (false == g_LpadcConversionCompletedFlag)
    {
    }
}

void DEMO_LPADC_IRQ_HANDLER_FUNC(void)
{
    g_CurrentTemperature           = DEMO_MeasureTemperature(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, 0U);
    g_LpadcConversionCompletedFlag = true;
    SDK_ISR_EXIT_BARRIER;
}


int fric_trim_test(void)
{
    int i, corse, fine;
    uint32_t targetFreq = 0UL;
    
    vref_config_t vrefConfig;
    VREF_GetDefaultConfig(&vrefConfig);
    /* Initialize the VREF mode. */
    VREF_Init(DEMO_VREF_BASE, &vrefConfig);
    /* Get a 1.8V reference voltage. */
    VREF_SetTrim21Val(DEMO_VREF_BASE, 8U);
    ADC_Configuration();
    
/*! SEL - Selects the CLKOUT clock source.
 *  0b0000..Main clock (main_clk)
 *  0b0001..PLL0 clock (pll0_clk)
 *  0b0010..CLKIN clock (clk_in)
 *  0b0011..FRO_HF clock (fro_hf)
 *  0b0100..FRO 12 MHz clock (fro_12m)
 *  0b0101..PLL1_clk0 clock (pll1_clk)
 *  0b0110..LP Oscillator clock (lp_osc)
 *  0b0111..USB PLL clock (usb_pll_clk)
*/
    SYSCON->CLKOUTSEL = SYSCON_CLKOUTSEL_SEL(3);
    SYSCON->CLKOUTDIV = 0; /* div by 4 */

    
    CLOCK_SetupFROHFClocking(144000000U);
    
    rt_kprintf("kCLOCK_DivClkOut:%d\r\n", CLOCK_GetFreq(kCLOCK_DivClkOut));
    
    INPUTMUX_Init(INPUTMUX);
    
    INPUTMUX->FREQMEAS_REF = kINPUTMUX_ClkInToFreqmeasRef;
    INPUTMUX->FREQMEAS_TAR = kINPUTMUX_Fro144MToFreqmeasTar;
    
    freq_measure_config_t config;
    FREQME_GetDefaultConfig(&config);
    config.operateModeAttribute.refClkScaleFactor = 25;
    FREQME_Init(FREQME0, &config);
        
    firc_trim_config_t trim_config;
    trim_config.trimMode = kSCG_FircTrimUpdate;
    trim_config.trimSrc = kSCG_FircTrimSrcSysOsc;
    trim_config.trimDiv = 23;
    
    CLOCK_FROHFTrimConfig(trim_config);
   
    rt_kprintf("FIRCSTAT:0x%X\r\n", SCG0->FIRCSTAT);
    rt_kprintf("FIRCSTAT:0x%X\r\n", SCG0->FIRCSTAT);

    SCG0->FIRCCSR &= ~SCG_FIRCCSR_LK_MASK;
    SCG0->FIRCCSR |= SCG_FIRCCSR_FIRCTREN_MASK;
    SCG0->FIRCCSR |= SCG_FIRCCSR_FIRCTRUP_MASK;


    rt_kprintf("FIRCTCFG[TRIMSRC]:%d\r\n", (SCG0->FIRCTCFG & SCG_FIRCTCFG_TRIMSRC_MASK) >> SCG_FIRCTCFG_TRIMSRC_SHIFT);
    rt_kprintf("FIRCTCFG[TRIMDIV]:%d\r\n", (SCG0->FIRCTCFG & SCG_FIRCTCFG_TRIMDIV_MASK) >> SCG_FIRCTCFG_TRIMDIV_SHIFT);


    printf("FRQ,ERROR,CORSE,FINE,TEMP\r\n");
    
    while (1)
    {
        FREQME_StartMeasurementCycle(FREQME0);
        targetFreq = FREQME_CalculateTargetClkFreq(FREQME0, CLOCK_GetFreq(kCLOCK_ExtClk));

        corse = (SCG0->FIRCSTAT & SCG_FIRCSTAT_TRIMCOAR_MASK) >> SCG_FIRCSTAT_TRIMCOAR_SHIFT;
        fine = (SCG0->FIRCSTAT & SCG_FIRCSTAT_TRIMFINE_MASK) >> SCG_FIRCSTAT_TRIMFINE_SHIFT;

        g_LpadcConversionCompletedFlag = false;
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (false == g_LpadcConversionCompletedFlag) {};

        printf("%f,%.2f%,%d,%d,%.1f\r\n", (float)targetFreq/(1000*1000), 100*ABS((double)144*1000 - (double)targetFreq/1000)/(144*1000), corse, fine,g_CurrentTemperature);
        
        rt_thread_mdelay(100);
    }
}


#ifdef RT_USING_FINSH
#include <finsh.h>
MSH_CMD_EXPORT(fric_trim_test, the cmd test);
#endif
