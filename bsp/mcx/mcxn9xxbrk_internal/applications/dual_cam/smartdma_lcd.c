
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_clock.h"
#include "rtthread.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MCXN_SAMRTDMA_CON_BASE   0x40033020
#define MCXN_SAMRTDMA_B0         ((SmartDMA_ARCH_B_CON_Type *)MCXN_SAMRTDMA_CON_BASE)
#define SAMRTDMA_STACK_SIZE          8
#define SAMRTDMA_SYNC_EN_SHIFT  (4U)

/*******************************************************************************
 * Variables
 ******************************************************************************/
typedef struct {
    uint32_t stack[SAMRTDMA_STACK_SIZE];     //stack memory for EZH
    uint32_t input_data_addr;               //address for lcd data
    uint32_t output_data_addr;                //data number
    uint32_t input_data_bytesize;                //data number
} cp_env_t;
cp_env_t cp_env;                        //environemnt vaiable for co-processor




//ezh_code[256]@ 0x20060000
uint32_t ezh_code_gray_rgb_code[64] =
{
0x00000012, 0x00000012, 0x33040404, 0x02180410, 0x02100410, 0x0FF40000, 0x0FF62C00, 0x0002EC0C, 0x008775A8, 0x00000012, 0x00000012, 0x08044001, 0x09044801, 0x0A045001, 0x01481010, 0x00151008,
0x03074C06, 0x0040C01A, 0x00000012, 0x01181401, 0x03581410, 0x46515410, 0x4B515410, 0x01180C01, 0x03380C10, 0x4630CC10, 0x4B30CC10, 0x10B0CC0D, 0x03B15416, 0x015C8402, 0x0C0E1000, 0x0DEC0C00,
0x0031100C, 0x33024C04, 0x0031100C, 0x21011018, 0x33402405, 0x20060023, 0x00004770
};

uint32_t ezh_code_gray_rgb_exe_buf[64] __attribute__ ((section(".ARM.__at_0x20060000")));


typedef struct {
  __IO uint32_t     EZHB_BOOT;
  __IO uint32_t     EZHB_CTRL;
  __I  uint32_t     reserve1;
  __I  uint32_t     reserve8;
  __IO uint32_t     reserve2;
  __IO uint32_t     reserve3;
  __IO uint32_t     reserve4;
  __IO uint32_t     reserve5;
  __IO uint32_t     EZHB_ARM2EZH;
  __IO uint32_t     reserve6;
  __IO uint32_t     reserve7;
 } SmartDMA_ARCH_B_CON_Type;

/*******************************************************************************
 * Code
 ******************************************************************************/
 void SmartDMA_Init(void)
 {
     //CP_InitLcdPin();
     CLOCK_EnableClock(kCLOCK_SmartDma);                //Enable clock for EZH
     MCXN_SAMRTDMA_B0->EZHB_CTRL = 0xC0DE0000 | (1UL << SAMRTDMA_SYNC_EN_SHIFT); //[31:16] must be 0xC0DE, [15:5] Reserved, must be 0, [4] must be 1 for AHB Synchorization
     MCXN_SAMRTDMA_B0->EZHB_ARM2EZH = (uint32_t)&cp_env;    //EZHB_ARM2EZH R/W,When [1:0]==0b10, EZH write to EZH2ARM trigger an interrupt to ARM, [31:2] General purpose
 }

void SmartDMA_Boot(void)
{
	MCXN_SAMRTDMA_B0->EZHB_BOOT = (uint32_t)ezh_code_gray_rgb_code;    //Pass the EZH program start address (must be 4-byte aligned)
	MCXN_SAMRTDMA_B0->EZHB_CTRL = 0xC0DE0011;                  //Set start bit of EZH
}

void SmartDMA_PrepareData(const uint32_t *pData_in, const uint32_t *pData_out, uint32_t databytesize)
{
    cp_env.input_data_addr =(uint32_t)pData_in;
    cp_env.output_data_addr = (uint32_t)pData_out;
    cp_env.input_data_bytesize = databytesize;
}
void SmartDMA_StartFormatConvert(void)
{
	MCXN_SAMRTDMA_B0->EZHB_CTRL = (0xC0DE0011 | (1<<1));     //set EX flag
}

void SmartDMA_WaitConvertDone(void)
{
    while(MCXN_SAMRTDMA_B0->EZHB_CTRL & (1<<1));
}




void smart_dma_g2rgb_run(void *input, void *output, uint32_t size)
{
	 SmartDMA_PrepareData(input, output, size);
	 /* Set EX flag to let EZH start to refresh */
	 SmartDMA_StartFormatConvert();
	 /* Wait EZH refresh complete */
	 SmartDMA_WaitConvertDone();
}

void smart_dma_g2rgb_init(void)
{
    memcpy(ezh_code_gray_rgb_exe_buf, ezh_code_gray_rgb_code, sizeof(ezh_code_gray_rgb_code));
    
    SmartDMA_Init();
    SmartDMA_Boot();
}





