

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"


void BOARD_InitQuadSPIPins(void);
void BOARD_InitI2C0Pins(void);
void BOARD_InitVSYNCPins(void);


void BOARD_InitBootPins(void)
{
    CLOCK_EnableClock(kCLOCK_Port0);  
    CLOCK_EnableClock(kCLOCK_Port1);
    CLOCK_EnableClock(kCLOCK_Port2);
    CLOCK_EnableClock(kCLOCK_Port3);
    CLOCK_EnableClock(kCLOCK_Port4);

    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Gpio2);
    CLOCK_EnableClock(kCLOCK_Gpio3);
    CLOCK_EnableClock(kCLOCK_Gpio4);    
    
    
    BOARD_InitPins();
    LPSPI1_InitPins();
    BOARD_InitQuadSPIPins();
    BOARD_InitI2C0Pins();
    BOARD_InitVSYNCPins();
}

void BOARD_InitPins(void)
{
    /* Enables the clock for PORT1: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port0);
    CLOCK_EnableClock(kCLOCK_Port1);
    CLOCK_EnableClock(kCLOCK_Port2);
    
    const port_pin_config_t port1_8_pinA1_config = {/* Internal pull-up/down resistor is disabled */
                                                    kPORT_PullDisable,
                                                    /* Low internal pull resistor value is selected. */
                                                    kPORT_LowPullResistor,
                                                    /* Fast slew rate is configured */
                                                    kPORT_FastSlewRate,
                                                    /* Passive input filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain output is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as FC4_P0 */
                                                    kPORT_MuxAlt2,
                                                    /* Digital input enabled */
                                                    kPORT_InputBufferEnable,
                                                    /* Digital input is not inverted */
                                                    kPORT_InputNormal,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORT1_8 (pin A1) is configured as FC4_P0 */
    PORT_SetPinConfig(PORT1, 8U, &port1_8_pinA1_config);

    const port_pin_config_t port1_9_pinB1_config = {/* Internal pull-up/down resistor is disabled */
                                                    kPORT_PullDisable,
                                                    /* Low internal pull resistor value is selected. */
                                                    kPORT_LowPullResistor,
                                                    /* Fast slew rate is configured */
                                                    kPORT_FastSlewRate,
                                                    /* Passive input filter is disabled */
                                                    kPORT_PassiveFilterDisable,
                                                    /* Open drain output is disabled */
                                                    kPORT_OpenDrainDisable,
                                                    /* Low drive strength is configured */
                                                    kPORT_LowDriveStrength,
                                                    /* Pin is configured as FC4_P1 */
                                                    kPORT_MuxAlt2,
                                                    /* Digital input enabled */
                                                    kPORT_InputBufferEnable,
                                                    /* Digital input is not inverted */
                                                    kPORT_InputNormal,
                                                    /* Pin Control Register fields [15:0] are not locked */
                                                    kPORT_UnlockRegister};
    /* PORT1_9 (pin B1) is configured as FC4_P1 */
    PORT_SetPinConfig(PORT1, 9U, &port1_9_pinB1_config);
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
LPSPI1_InitPins:
- options: {callFromInitBoot: 'true', coreID: cm33_core0, enableClock: 'true'}
- pin_list:
  - {pin_num: B6, peripheral: LPFlexcomm1, signal: LPFLEXCOMM_P0, pin_signal: PIO0_24/FC1_P0/CT0_MAT0/ADC0_B16, slew_rate: slow, open_drain: disable, drive_strength: low,
    pull_select: up, pull_enable: enable, input_buffer: enable, invert_input: normal}
  - {pin_num: A6, peripheral: LPFlexcomm1, signal: LPFLEXCOMM_P1, pin_signal: PIO0_25/FC1_P1/CT0_MAT1/ADC0_B17, slew_rate: slow, open_drain: disable, drive_strength: low,
    pull_select: up, pull_enable: enable, input_buffer: enable, invert_input: normal}
  - {pin_num: F10, peripheral: LPFlexcomm1, signal: LPFLEXCOMM_P2, pin_signal: PIO0_26/FC1_P2/CT0_MAT2/ADC0_B18, slew_rate: slow, open_drain: disable, drive_strength: low,
    pull_select: up, pull_enable: enable, input_buffer: enable, invert_input: normal}
  - {pin_num: E10, peripheral: LPFlexcomm1, signal: LPFLEXCOMM_P3, pin_signal: PIO0_27/FC1_P3/CT0_MAT3/ADC0_B19, slew_rate: slow, open_drain: disable, drive_strength: low,
    pull_select: up, pull_enable: enable, input_buffer: enable, invert_input: normal}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : LPSPI1_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void LPSPI1_InitPins(void)
{
    /* Enables the clock for PORT0 controller: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port0);

    const port_pin_config_t port0_24_pinB6_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Low internal pull resistor value is selected. */
                                                     kPORT_LowPullResistor,
                                                     /* Slow slew rate is configured */
                                                     kPORT_SlowSlewRate,
                                                     /* Passive input filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as FC1_P0 */
                                                     kPORT_MuxAlt2,
                                                     /* Digital input enabled */
                                                     kPORT_InputBufferEnable,
                                                     /* Digital input is not inverted */
                                                     kPORT_InputNormal,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORT0_24 (pin B6) is configured as FC1_P0 */
    PORT_SetPinConfig(PORT0, 24U, &port0_24_pinB6_config);

    const port_pin_config_t port0_25_pinA6_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Low internal pull resistor value is selected. */
                                                     kPORT_LowPullResistor,
                                                     /* Slow slew rate is configured */
                                                     kPORT_SlowSlewRate,
                                                     /* Passive input filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as FC1_P1 */
                                                     kPORT_MuxAlt2,
                                                     /* Digital input enabled */
                                                     kPORT_InputBufferEnable,
                                                     /* Digital input is not inverted */
                                                     kPORT_InputNormal,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORT0_25 (pin A6) is configured as FC1_P1 */
    PORT_SetPinConfig(PORT0, 25U, &port0_25_pinA6_config);

    const port_pin_config_t port0_26_pinF10_config = {/* Internal pull-up resistor is enabled */
                                                      kPORT_PullUp,
                                                      /* Low internal pull resistor value is selected. */
                                                      kPORT_LowPullResistor,
                                                      /* Slow slew rate is configured */
                                                      kPORT_SlowSlewRate,
                                                      /* Passive input filter is disabled */
                                                      kPORT_PassiveFilterDisable,
                                                      /* Open drain output is disabled */
                                                      kPORT_OpenDrainDisable,
                                                      /* Low drive strength is configured */
                                                      kPORT_LowDriveStrength,
                                                      /* Pin is configured as FC1_P2 */
                                                      kPORT_MuxAlt2,
                                                      /* Digital input enabled */
                                                      kPORT_InputBufferEnable,
                                                      /* Digital input is not inverted */
                                                      kPORT_InputNormal,
                                                      /* Pin Control Register fields [15:0] are not locked */
                                                      kPORT_UnlockRegister};
    /* PORT0_26 (pin F10) is configured as FC1_P2 */
    PORT_SetPinConfig(PORT0, 26U, &port0_26_pinF10_config);

    const port_pin_config_t port0_27_pinE10_config = {/* Internal pull-up resistor is enabled */
                                                      kPORT_PullUp,
                                                      /* Low internal pull resistor value is selected. */
                                                      kPORT_LowPullResistor,
                                                      /* Slow slew rate is configured */
                                                      kPORT_SlowSlewRate,
                                                      /* Passive input filter is disabled */
                                                      kPORT_PassiveFilterDisable,
                                                      /* Open drain output is disabled */
                                                      kPORT_OpenDrainDisable,
                                                      /* Low drive strength is configured */
                                                      kPORT_LowDriveStrength,
                                                      /* Pin is configured as FC1_P3 */
                                                      kPORT_MuxAlt2,
                                                      /* Digital input enabled */
                                                      kPORT_InputBufferEnable,
                                                      /* Digital input is not inverted */
                                                      kPORT_InputNormal,
                                                      /* Pin Control Register fields [15:0] are not locked */
                                                      kPORT_UnlockRegister};
    /* PORT0_27 (pin E10) is configured as FC1_P3 */
    PORT_SetPinConfig(PORT0, 27U, &port0_27_pinE10_config);
}


void BOARD_InitI2C0Pins(void)
{
     /*I2C0 pin configuration*/    
    const port_pin_config_t port0_16_pinP0_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Low internal pull resistor value is selected. */
                                                     kPORT_LowPullResistor,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive input filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as PIO0_16 */
                                                     kPORT_MuxAlt2,
                                                     /* Digital input enabled */
                                                     kPORT_InputBufferEnable,
                                                     /* Digital input is not inverted */
                                                     kPORT_InputNormal,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORT0_16 (pin P0) is configured as I2C_SDA */
    PORT_SetPinConfig(PORT0, 16U, &port0_16_pinP0_config);
    
    const port_pin_config_t port0_17_pinP1_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Low internal pull resistor value is selected. */
                                                     kPORT_LowPullResistor,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive input filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as PIO0_17 */
                                                     kPORT_MuxAlt2,
                                                     /* Digital input enabled */
                                                     kPORT_InputBufferEnable,
                                                     /* Digital input is not inverted */
                                                     kPORT_InputNormal,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORT0_17 (pin P1) is configured as I2C_SCL */
    PORT_SetPinConfig(PORT0, 17U, &port0_17_pinP1_config);        
}

void BOARD_InitQuadSPIPins(void)
{
    /* Enables the clock for PORT1: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port1);

    /* PORT1_0 (pin C6) is configured as FC3_P0 */
    PORT_SetPinMux(PORT1, 0U, kPORT_MuxAlt2);

    PORT1->PCR[0] = ((PORT1->PCR[0] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_1 (pin C5) is configured as FC3_P1 */
    PORT_SetPinMux(PORT1, 1U, kPORT_MuxAlt2);

    PORT1->PCR[1] = ((PORT1->PCR[1] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_2 (pin C4) is configured as FC3_P2 */
    PORT_SetPinMux(PORT1, 2U, kPORT_MuxAlt2);

    PORT1->PCR[2] = ((PORT1->PCR[2] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_3 (pin B4) is configured as FC3_P3 */
    PORT_SetPinMux(PORT1, 3U, kPORT_MuxAlt2);

    PORT1->PCR[3] = ((PORT1->PCR[3] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_4 (pin A4) is configured as FC3_P4 */
    PORT_SetPinMux(PORT1, 4U, kPORT_MuxAlt2);

    PORT1->PCR[4] = ((PORT1->PCR[4] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_5 (pin B3) is configured as FC3_P5 */
    PORT_SetPinMux(PORT1, 5U, kPORT_MuxAlt2);

    PORT1->PCR[5] = ((PORT1->PCR[5] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_6 (pin B2) is configured as FC3_P6 */
    PORT_SetPinMux(PORT1, 6U, kPORT_MuxAlt2);

    PORT1->PCR[6] = ((PORT1->PCR[6] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));
}


void BOARD_InitVSYNCPins(void)
{
    const port_pin_config_t port0_15_config = {/* Internal pull-up resistor is enabled */
                                                     kPORT_PullUp,
                                                     /* Low internal pull resistor value is selected. */
                                                     kPORT_LowPullResistor,
                                                     /* Fast slew rate is configured */
                                                     kPORT_FastSlewRate,
                                                     /* Passive input filter is disabled */
                                                     kPORT_PassiveFilterDisable,
                                                     /* Open drain output is disabled */
                                                     kPORT_OpenDrainDisable,
                                                     /* Low drive strength is configured */
                                                     kPORT_LowDriveStrength,
                                                     /* Pin is configured as PIO0_29 */
                                                     kPORT_MuxAlt0,
                                                     /* Digital input enabled */
                                                     kPORT_InputBufferEnable,
                                                     /* Digital input is not inverted */
                                                     kPORT_InputNormal,
                                                     /* Pin Control Register fields [15:0] are not locked */
                                                     kPORT_UnlockRegister};
    /* PORT0_29 (pin F8) is configured as PIO0_29 */
    PORT_SetPinConfig(PORT0, 15, &port0_15_config);
}


/* Configure port mux of FlexIO data pins */
void FLEXIO_8080_Config_Data_Pin(void)
{
    FLEXIO_DATA0_PORT->PCR[FLEXIO_DATA0_PIN]   = PORT_PCR_MUX(FLEXIO_DATA0_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D0 */
    FLEXIO_DATA1_PORT->PCR[FLEXIO_DATA1_PIN]   = PORT_PCR_MUX(FLEXIO_DATA1_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D1 */
    FLEXIO_DATA2_PORT->PCR[FLEXIO_DATA2_PIN]   = PORT_PCR_MUX(FLEXIO_DATA2_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D2 */
    FLEXIO_DATA3_PORT->PCR[FLEXIO_DATA3_PIN]   = PORT_PCR_MUX(FLEXIO_DATA3_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D3 */
    FLEXIO_DATA4_PORT->PCR[FLEXIO_DATA4_PIN]   = PORT_PCR_MUX(FLEXIO_DATA4_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D4 */
    FLEXIO_DATA5_PORT->PCR[FLEXIO_DATA5_PIN]   = PORT_PCR_MUX(FLEXIO_DATA5_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D5 */
    FLEXIO_DATA6_PORT->PCR[FLEXIO_DATA6_PIN]   = PORT_PCR_MUX(FLEXIO_DATA6_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D6 */
    FLEXIO_DATA7_PORT->PCR[FLEXIO_DATA7_PIN]   = PORT_PCR_MUX(FLEXIO_DATA7_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D7 */

    FLEXIO_DATA8_PORT->PCR[FLEXIO_DATA8_PIN]   = PORT_PCR_MUX(FLEXIO_DATA8_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D8  */
    FLEXIO_DATA9_PORT->PCR[FLEXIO_DATA9_PIN]   = PORT_PCR_MUX(FLEXIO_DATA9_MUX)  | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D9  */
    FLEXIO_DATA10_PORT->PCR[FLEXIO_DATA10_PIN] = PORT_PCR_MUX(FLEXIO_DATA10_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D10 */
    FLEXIO_DATA11_PORT->PCR[FLEXIO_DATA11_PIN] = PORT_PCR_MUX(FLEXIO_DATA11_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D11 */
    FLEXIO_DATA12_PORT->PCR[FLEXIO_DATA12_PIN] = PORT_PCR_MUX(FLEXIO_DATA12_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D12 */
    FLEXIO_DATA13_PORT->PCR[FLEXIO_DATA13_PIN] = PORT_PCR_MUX(FLEXIO_DATA13_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D13 */
    FLEXIO_DATA14_PORT->PCR[FLEXIO_DATA14_PIN] = PORT_PCR_MUX(FLEXIO_DATA14_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D14 */
    FLEXIO_DATA15_PORT->PCR[FLEXIO_DATA15_PIN] = PORT_PCR_MUX(FLEXIO_DATA15_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS(0);       /* FXIO0_D15 */
}


/* Configure FLEXIO_WR pin as FlexIO function */
void FLEXIO_8080_Config_WR_FlexIO(void)
{
    FLEXIO_WR_PORT->PCR[FLEXIO_WR_PIN] = PORT_PCR_MUX(FLEXIO_WR_PIN_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Configure FLEXIO_WR pin as GPIO function and outputting high level */
void FLEXIO_8080_Config_WR_GPIO(void)
{
    FLEXIO_WR_GPIO->PSOR |= 1U << FLEXIO_WR_PIN;
    FLEXIO_WR_GPIO->PDDR |= 1U << FLEXIO_WR_PIN;
    FLEXIO_WR_PORT->PCR[FLEXIO_WR_PIN] = PORT_PCR_MUX(0U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Configure FLEXIO_RD pin as FlexIO function */
void FLEXIO_8080_Config_RD_FlexIO(void)
{
    FLEXIO_RD_PORT->PCR[FLEXIO_RD_PIN] = PORT_PCR_MUX(FLEXIO_RD_PIN_MUX) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Configure FLEXIO_RD pin as GPIO function and outputting high level */
void FLEXIO_8080_Config_RD_GPIO(void)
{
    FLEXIO_RD_GPIO->PSOR |= 1U << FLEXIO_RD_PIN;
    FLEXIO_RD_GPIO->PDDR |= 1U << FLEXIO_RD_PIN;
    FLEXIO_RD_PORT->PCR[FLEXIO_RD_PIN] = PORT_PCR_MUX(0U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Configure FLEXIO_CS pin as GPIO function and outputting high level */
void FLEXIO_8080_Config_CS_GPIO(void)
{
    FLEXIO_CS_GPIO->PSOR |= 1U << FLEXIO_CS_PIN;
    FLEXIO_CS_GPIO->PDDR |= 1U << FLEXIO_CS_PIN;
    FLEXIO_CS_PORT->PCR[FLEXIO_CS_PIN] = PORT_PCR_MUX(0U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Set FLEXIO_CS pin's level */
void FLEXIO_8080_Set_CS_Pin(bool level)
{
    if(level)
    {
        FLEXIO_CS_GPIO->PSOR |= 1U << FLEXIO_CS_PIN;
    }
    else
    {
        FLEXIO_CS_GPIO->PCOR |= 1U << FLEXIO_CS_PIN;
    }
}

/* Configure RS pin as GPIO function and outputting high level */
void FLEXIO_8080_Config_RS_GPIO(void)
{
    FLEXIO_RS_GPIO->PSOR |= 1U << FLEXIO_RS_PIN;
    FLEXIO_RS_GPIO->PDDR |= 1U << FLEXIO_RS_PIN;
    FLEXIO_RS_PORT->PCR[FLEXIO_RS_PIN] = PORT_PCR_MUX(0U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Set RS pin's level */
void FLEXIO_8080_Set_RS_Pin(bool level)
{
    if(level)
    {
        FLEXIO_RS_GPIO->PSOR |= 1U << FLEXIO_RS_PIN;
    }
    else
    {
        FLEXIO_RS_GPIO->PCOR |= 1U << FLEXIO_RS_PIN;
    }
}

/* Configure ReSet pin as GPIO function and outputting high level */
void FLEXIO_8080_Config_ReSet_GPIO(void)
{
    FLEXIO_ReSet_GPIO->PSOR |= 1U << FLEXIO_ReSet_PIN;
    FLEXIO_ReSet_GPIO->PDDR |= 1U << FLEXIO_ReSet_PIN;
    FLEXIO_ReSet_PORT->PCR[FLEXIO_ReSet_PIN] = PORT_PCR_MUX(0U) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
}

/* Set ReSet pin's level */
void FLEXIO_8080_Set_ReSet_Pin(bool level)
{
    if(level)
    {
        FLEXIO_ReSet_GPIO->PSOR |= 1U << FLEXIO_ReSet_PIN;
    }
    else
    {
        FLEXIO_ReSet_GPIO->PCOR |= 1U << FLEXIO_ReSet_PIN;
    }
}
