/*
 * Copyright (c) 2019-2020, Xim
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include <rthw.h>
#include <rtdevice.h>

#include "board.h"
#include "drv_uart.h"

#include <stdio.h>
#include "sbi.h"

#include <rtdbg.h>

#include <kconfig.h>

#ifdef CONFIG_SOC_SUN20IW1
#include <hal_uart.h>
#endif

#ifdef BOARD_allwinnerd1
#define UART_DEFAULT_BAUDRATE               115200
#else
#define UART_DEFAULT_BAUDRATE               500000
#endif

#define SUNXI_UART_ADDR            0x02500000

#define SUNXI_UART_RBR (0x00)       /* receive buffer register */
#define SUNXI_UART_THR (0x00)       /* transmit holding register */
#define SUNXI_UART_DLL (0x00)       /* divisor latch low register */
#define SUNXI_UART_DLH (0x04)       /* diviso latch high register */
#define SUNXI_UART_IER (0x04)       /* interrupt enable register */
#define SUNXI_UART_IIR (0x08)       /* interrupt identity register */
#define SUNXI_UART_FCR (0x08)       /* FIFO control register */
#define SUNXI_UART_LCR (0x0c)       /* line control register */
#define SUNXI_UART_MCR (0x10)       /* modem control register */
#define SUNXI_UART_LSR (0x14)       /* line status register */
#define SUNXI_UART_MSR (0x18)       /* modem status register */
#define SUNXI_UART_SCH (0x1c)       /* scratch register */
#define SUNXI_UART_USR (0x7c)       /* status register */
#define SUNXI_UART_TFL (0x80)       /* transmit FIFO level */
#define SUNXI_UART_RFL (0x84)       /* RFL */
#define SUNXI_UART_HALT (0xa4)      /* halt tx register */
#define SUNXI_UART_RS485 (0xc0)     /* RS485 control and status register */

#define _BIT(x) (1 << x)

/* Line Status Rigster */
#define SUNXI_UART_LSR_RXFIFOE    (_BIT(7))
#define SUNXI_UART_LSR_TEMT       (_BIT(6))
#define SUNXI_UART_LSR_THRE       (_BIT(5))
#define SUNXI_UART_LSR_BI         (_BIT(4))
#define SUNXI_UART_LSR_FE         (_BIT(3))
#define SUNXI_UART_LSR_PE         (_BIT(2))
#define SUNXI_UART_LSR_OE         (_BIT(1))
#define SUNXI_UART_LSR_DR         (_BIT(0))
#define SUNXI_UART_LSR_BRK_ERROR_BITS 0x1E /* BI, FE, PE, OE _BITs */

struct device_uart
{
    rt_ubase_t  hw_base;
    rt_uint32_t irqno;
#ifdef CONFIG_SOC_SUN20IW1
    uart_port_t uart_port;
#endif
};

static rt_err_t  rt_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg);
static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg);
static int       drv_uart_putc(struct rt_serial_device *serial, char c);
static int       drv_uart_getc(struct rt_serial_device *serial);

const struct rt_uart_ops _uart_ops =
{
    rt_uart_configure,
    uart_control,
    drv_uart_putc,
    drv_uart_getc,
    //TODO: add DMA support
    RT_NULL
};

void uart_init(void)
{
    return ;
}

struct rt_serial_device  serial1;
struct device_uart       uart1;

#ifdef CONFIG_SOC_SUN20IW1
/* hal uart map table */
static const uint32_t g_uart_baudrate_map[] =
{
    300,
    600,
    1200,
    2400,
    4800,
    9600,
    19200,
    38400,
    57600,
    115200,
    230400,
    576000,
    921600,
    500000,
    1000000,
    1500000,
    3000000,
    4000000,
};

static int get_hal_baudrate(rt_uint32_t baudrate)
{
    int i;
    for (i = 0; i < sizeof(g_uart_baudrate_map) / sizeof(uint32_t); i++)
    {
        if (baudrate == g_uart_baudrate_map[i])
        {
            return i;   /* index of hal baudrate */
        }
    }
    LOG_E("uart: not support baudrate:%d!", baudrate);
    return -1;
}

static int get_hal_word_length(rt_uint32_t data_bits)
{
    int word_len = -1;
    switch (data_bits)
    {
    case DATA_BITS_5:
        word_len = UART_WORD_LENGTH_5;
        break;
    case DATA_BITS_6:
        word_len = UART_WORD_LENGTH_6;
        break;
    case DATA_BITS_7:
        word_len = UART_WORD_LENGTH_7;
        break;
    case DATA_BITS_8:
        word_len = UART_WORD_LENGTH_8;
        break;
    default:
        LOG_E("uart: not support data bits:%d!", data_bits);
        break;
    }
    return word_len;
}

static int get_hal_stop_bits(rt_uint32_t stop_bits)
{
    int hal_stop_bits = -1;
    /* set stop bit */
    switch (stop_bits)
    {
    case STOP_BITS_1:
        hal_stop_bits = UART_STOP_BIT_1;
        break;
    case STOP_BITS_2:
        hal_stop_bits = UART_STOP_BIT_2;
        break;
    default:
        LOG_E("uart: not support stop bits:%d!", stop_bits);
        break;
    }
    return hal_stop_bits;
}

static int get_hal_parity(rt_uint32_t parity)
{
    /* set parity bit */
    int hal_parity = -1;
    switch (parity)
    {
    case PARITY_NONE:
        hal_parity = UART_PARITY_NONE;
        break;
    case PARITY_ODD:
        hal_parity = UART_PARITY_ODD;
        break;
    case PARITY_EVEN:
        hal_parity = UART_PARITY_EVEN;
        break;
    default:
        LOG_E("uart: not support parity:%d!", parity);
        break;
    }
    return hal_parity;
}
#endif /* CONFIG_SOC_SUN20IW1 */

/*
 * UART interface
 */
static rt_err_t rt_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct device_uart *uart;

    RT_ASSERT(serial != RT_NULL);

    serial->config = *cfg;
    uart = serial->parent.user_data;

#ifdef CONFIG_SOC_SUN20IW1
    _uart_config_t hal_cfg;

    hal_cfg.baudrate = get_hal_baudrate(cfg->baud_rate);
    if (hal_cfg.baudrate == -1)
    {
        return -RT_EINVAL;
    }
    hal_cfg.word_length = get_hal_word_length(cfg->data_bits);
    if (hal_cfg.word_length == -1)
    {
        return -RT_EINVAL;
    }
    hal_cfg.stop_bit = get_hal_stop_bits(cfg->stop_bits);
    if (hal_cfg.stop_bit == -1)
    {
        return -RT_EINVAL;
    }
    hal_cfg.parity = get_hal_parity(cfg->parity);
    if (hal_cfg.parity == -1)
    {
        return -RT_EINVAL;
    }
    hal_uart_control(uart->uart_port, 0, &hal_cfg); /* translate cfg to hal cfg */
#endif /* CONFIG_SOC_SUN20IW1 */
    return (RT_EOK);
}

static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct device_uart *uart;

    uart = serial->parent.user_data;
    rt_uint32_t channel = 1;

    RT_ASSERT(uart != RT_NULL);
    RT_ASSERT(channel != 3);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        break;

    case RT_DEVICE_CTRL_SET_INT:
        break;
    }

    return (RT_EOK);
}

static int drv_uart_putc(struct rt_serial_device *serial, char c)
{
#ifndef CONFIG_SOC_SUN20IW1
    sbi_console_putchar(c);
#else
    hal_uart_put_char(0, c);
#endif
    return (1);
}

static int drv_uart_getc(struct rt_serial_device *serial)
{
#ifndef CONFIG_SOC_SUN20IW1
    uint32_t *lsr = (uint32_t *)(SUNXI_UART_ADDR + SUNXI_UART_LSR);
    uint32_t *rbr = (uint32_t *)(SUNXI_UART_ADDR + SUNXI_UART_RBR);

    if(!(*lsr & SUNXI_UART_LSR_DR))
    {
        return -1;
    }
    return (int)*rbr;
#else
    int data;
    int recv_count;
    recv_count = hal_uart_receive_no_block(0,(uint8_t *)&data,1,0);
    if(!recv_count)
    {
        data = -1;
    }

    return data;
#endif
}

#ifdef CONFIG_SOC_SUN20IW1
/**
 * this function will called when uart interrupt occur!
 */
void hal_uart_handler_hook(uart_port_t uart_port)
{
    if (uart_port == 0) /* uart0 */
    {
        rt_hw_serial_isr((struct rt_serial_device *)&serial1,RT_SERIAL_EVENT_RX_IND);
    }
}
#endif

/*
 * UART Initiation
 */
int rt_hw_uart_init(void)
{
    struct rt_serial_device *serial;
    struct device_uart      *uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    serial  = &serial1;
    uart    = &uart1;

    serial->ops              = &_uart_ops;
    serial->config           = config;
    serial->config.baud_rate = UART_DEFAULT_BAUDRATE;

    uart->hw_base   = 0x10000000;
    uart->irqno     = 0xa;
#ifdef CONFIG_SOC_SUN20IW1
    uart->uart_port = 0;
#endif

    rt_hw_serial_register(serial,
                          "uart",
                          RT_DEVICE_FLAG_STREAM | RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);

    return 0;
}
