
#include "fsl_common.h"
#include "fsl_flexcan.h"
#include "app_drv_can.h"


/*
 *    DWORD_IN_MB    DLC    BYTES_IN_MB             Maximum MBs
 *    2              8      kFLEXCAN_8BperMB        64
 *    4              10     kFLEXCAN_16BperMB       42
 *    8              13     kFLEXCAN_32BperMB       25
 *    16             15     kFLEXCAN_64BperMB       14
 *
 * Dword in each message buffer, Length of data in bytes, Payload size must align,
 * and the Message Buffers are limited corresponding to each payload configuration:
 */



#define EXAMPLE_CAN         CAN0
#define RX_MB_IDX           (0)
#define TX_MB_IDX           (1)


static flexcan_handle_t flexcanHandle;
static flexcan_mb_transfer_t rxXfer;
static flexcan_fd_frame_t fd_rx_frame;
static flexcan_frame_t rx_frame;
static app_can_event_cb_t can_event_cb = NULL;
static uint8_t is_fd = 0;

static uint8_t dlc2len(uint8_t dlc)
{
    if (dlc < 9) {
        return dlc;
    } else if (dlc == 9) {
        return 12;
    } else if (dlc == 10) {
        return 16;
    } else if (dlc == 11) {
        return 20;
    } else if (dlc == 12) {
        return 24;
    } else if (dlc == 13) {
        return 32;
    } else if (dlc == 14) {
        return 48;
    } else if (dlc == 15) {
        return 64;
    } else {

        return 0;
    }
}

static uint8_t len2dlc(uint8_t len)
{
    if (len <= 8) {
        return len;
    } else if (len <= 12) {
        return 9;
    } else if (len <= 16) {
        return 10;
    } else if (len <= 20) {
        return 11;
    } else if (len <= 24) {
        return 12;
    } else if (len <= 32) {
        return 13;
    } else if (len <= 48) {
        return 14;
    } else if (len <= 64) {
        return 15;
    } else {

        return 0xFF; 
    }
}

static void reverse_byte_order(const uint8_t* input, uint8_t* output, size_t len)
{
    for (size_t i = 0; i < len; i += 4)
    {
        output[i] = input[i + 3];
        output[i + 1] = input[i + 2];
        output[i + 2] = input[i + 1];
        output[i + 3] = input[i];
    }
}

static void fd_frame_decode(flexcan_fd_frame_t *frame, uint8_t *buf, uint8_t *dlc, uint32_t *id)
{
    *dlc = frame->length;
    *id = (frame->id & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
    reverse_byte_order((uint8_t*)frame->dataWord, buf, dlc2len(*dlc));
}

static void frame_decode(flexcan_frame_t *frame, uint8_t *buf, uint8_t *dlc, uint32_t *id)
{
    *dlc = frame->length;
    *id = (frame->id & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
    reverse_byte_order((uint8_t*)&frame->dataWord0, buf, dlc2len(*dlc));
}

#include <rtthread.h>

static void flexcan_callback(CAN_Type *base, flexcan_handle_t *handle, status_t status, uint64_t result, void *userData)
{
    int i;
    uint8_t buf[64];
    uint8_t dlc;
    uint32_t id;
    
    switch (status)
    {
        case kStatus_FLEXCAN_RxIdle:
            if (RX_MB_IDX == result)
            {
                if(is_fd)
                {
                    fd_frame_decode(&fd_rx_frame, buf, &dlc, &id);
                    can_event_cb(id, buf, dlc2len(dlc));
                    FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
                }
                else
                {
                    frame_decode(&rx_frame, buf, &dlc, &id);
                    can_event_cb(id, buf, dlc2len(dlc));
                    FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
                }
            }

            break;
        case kStatus_FLEXCAN_TxIdle:
                //rt_kprintf("kStatus_FLEXCAN_TxIdle\r\n");
            break;

        case kStatus_FLEXCAN_WakeUp:
                //rt_kprintf("kStatus_FLEXCAN_WakeUp\r\n");
            break;
        case kStatus_FLEXCAN_ErrorStatus:
                rt_kprintf("kStatus_FLEXCAN_ErrorStatus\r\n");
            break;
        case kStatus_FLEXCAN_RxOverflow:
                rt_kprintf("kStatus_FLEXCAN_RxOverflow\r\n");
            break;
        case kStatus_FLEXCAN_RxFifoOverflow:
                rt_kprintf("kStatus_FLEXCAN_RxFifoOverflow\r\n");
            break;
        default:
            break;
    }
}


void app_can_add_event_cb(app_can_event_cb_t event_cb, uint8_t event_code, void* user_data)
{
    can_event_cb = event_cb;
}

void app_can_init(uint32_t baud, uint32_t baudfd, uint8_t fd)
{
    int i;
    is_fd = fd;
    flexcan_config_t flexcanConfig;
    flexcan_rx_mb_config_t mbConfig;

    CLOCK_SetClkDiv(kCLOCK_DivFlexcan0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_FLEXCAN0);

    FLEXCAN_GetDefaultConfig(&flexcanConfig);
    flexcanConfig.baudRate = baud;
    flexcanConfig.baudRateFD = baudfd;

    flexcan_timing_config_t timing_config;
    memset(&timing_config, 0, sizeof(flexcan_timing_config_t));

    /* Setup Rx Message Buffer. */
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.type   = kFLEXCAN_FrameTypeData;
    mbConfig.id     = FLEXCAN_ID_STD(0x456);
    
    if(is_fd)
    {
        FLEXCAN_FDCalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.bitRate, flexcanConfig.bitRateFD, CLOCK_GetFlexcanClkFreq(0U), &timing_config);
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
        FLEXCAN_FDInit(EXAMPLE_CAN, &flexcanConfig, CLOCK_GetFlexcanClkFreq(0U), kFLEXCAN_64BperMB, true);
        FLEXCAN_SetFDRxMbConfig(EXAMPLE_CAN, RX_MB_IDX, &mbConfig, true);
        rxXfer.framefd = &fd_rx_frame;
    }
    else
    {
        FLEXCAN_CalculateImprovedTimingValues(EXAMPLE_CAN, flexcanConfig.baudRate, CLOCK_GetFlexcanClkFreq(0), &timing_config);
        memcpy(&(flexcanConfig.timingConfig), &timing_config, sizeof(flexcan_timing_config_t));
        FLEXCAN_Init(EXAMPLE_CAN, &flexcanConfig, CLOCK_GetFlexcanClkFreq(0U));
        FLEXCAN_SetRxMbConfig(EXAMPLE_CAN, RX_MB_IDX, &mbConfig, true);
        rxXfer.frame = &rx_frame;
    }
    
    FLEXCAN_SetRxMbGlobalMask(EXAMPLE_CAN, FLEXCAN_RX_MB_STD_MASK(0xFF0, 0, 0));
    FLEXCAN_TransferCreateHandle(EXAMPLE_CAN, &flexcanHandle, flexcan_callback, NULL);

    rxXfer.mbIdx = (uint8_t)RX_MB_IDX;
    
    if(is_fd)
    {
        FLEXCAN_TransferFDReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
    }
    else
    {
        FLEXCAN_TransferReceiveNonBlocking(EXAMPLE_CAN, &flexcanHandle, &rxXfer);
    }
}


int app_can_send(uint8_t *buf, uint32_t id, uint8_t len)
{
    status_t status;
    
    if(is_fd)
    {
        flexcan_fd_frame_t fd_txFrame = {0};
        fd_txFrame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
        fd_txFrame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
        fd_txFrame.id = FLEXCAN_ID_STD(id);
        fd_txFrame.length = len2dlc(len);
        fd_txFrame.brs = 1U;
        
        reverse_byte_order(buf, (uint8_t*)fd_txFrame.dataWord, len);
        FLEXCAN_SetFDTxMbConfig(EXAMPLE_CAN, TX_MB_IDX, true);
        status = FLEXCAN_TransferFDSendBlocking(EXAMPLE_CAN, TX_MB_IDX, &fd_txFrame);
    }
    else
    {
        flexcan_frame_t txFrame = {0};
        txFrame.format = (uint8_t)kFLEXCAN_FrameFormatStandard;
        txFrame.type   = (uint8_t)kFLEXCAN_FrameTypeData;
        txFrame.id = FLEXCAN_ID_STD(id);
        txFrame.length = len2dlc(len);
        
        reverse_byte_order(buf, (uint8_t*)&txFrame.dataWord0, len);
        FLEXCAN_SetTxMbConfig(EXAMPLE_CAN, TX_MB_IDX, true);
        status = FLEXCAN_TransferSendBlocking(EXAMPLE_CAN, TX_MB_IDX, &txFrame);
    }
    
    return (status == kStatus_Success)?(0):(1);
}



