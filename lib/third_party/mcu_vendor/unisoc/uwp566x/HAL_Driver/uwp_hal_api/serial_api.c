/*
 * Copyright (c) 2018, UNISOC Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "serial_api.h"
#include "uwp_hal.h"
#include "UWP_5661.h"

#define UART_STRUCT(obj) \
    ((volatile struct uwp_uart *)(obj->base))

#define DIV_ROUND(n, d) (((n) + ((d)/2)) / (d))

serial_t stdio_uart = {0x40038000, 26000000, 115200};
int stdio_uart_inited = 0;

#define UART_NUM    2
static uart_irq_handler irq_handler[UART_NUM] = {NULL};

/** zephyr reference */

static inline int uart_uwp_poll_in(serial_t *obj, unsigned char *c)
{
    volatile struct uwp_uart *uart = UART_STRUCT(obj);

	if (uwp_uart_rx_ready(uart)) {
		*c = uwp_uart_read(uart);
		return 0;
	}

    return -1;
}

static inline int uart_uwp_init(serial_t *obj)
{
    volatile struct uwp_uart *uart = UART_STRUCT(obj);
    if(!stdio_uart_inited){
        uwp_sys_enable(BIT(APB_EB_UART0));
        uwp_sys_reset(BIT(APB_EB_UART0));

        uwp_uart_set_cdk(uart, DIV_ROUND(obj->sys_clk_freq, obj->baud_rate));
        uwp_uart_set_stop_bit_num(uart, 1);
        uwp_uart_set_byte_len(uart, 3);

        uwp_uart_init(uart);
        stdio_uart_inited = 1;
    }
    else{
        
    }

    return 0;
}


#if 0
#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static void uart_uwp_isr(void *arg);
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

static inline unsigned char uart_uwp_poll_out(struct device *dev, unsigned char c)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    if (malin_uart_tx_ready(uart)) {
        uwp_uart_write(uart, c);
        while (!uwp_uart_trans_over(uart));

        return 0;
    }

    return -1;
}

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
static int uart_uwp_fifo_fill(struct device *dev, const u8_t *tx_data,
                 int size)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);
    unsigned int num_tx = 0;

    while ((size - num_tx) > 0) {
        /* Send a character */
        if (malin_uart_tx_ready(uart)) {
            uwp_uart_write(uart, tx_data[num_tx]);
            num_tx++;
        } else {
            break;
        }
    }

    return (int)num_tx;
}

static int uart_uwp_fifo_read(struct device *dev, u8_t *rx_data,
                 const int size)
{
    unsigned int num_rx = 0;
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    while (((size - num_rx) > 0) &&
            malin_uart_rx_ready(uart)) {

    /* Receive a character */
    rx_data[num_rx++] = uwp_uart_read(uart);
}

    return num_rx;
}

static void uart_uwp_irq_tx_enable(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    uwp_uart_int_enable(uart, UART_TXF_EMPTY);
}

static void uart_uwp_irq_tx_disable(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    uwp_uart_int_disable(uart, UART_TXF_EMPTY);
}

static int uart_uwp_irq_tx_ready(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);
    u32_t status;

    status = uwp_uart_status(uart);

    return (status & UART_TXF_EMPTY);
}

static void uart_uwp_irq_rx_enable(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    uwp_uart_int_enable(uart, BIT(UART_RXF_FULL));
}

static void uart_uwp_irq_rx_disable(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    uwp_uart_int_disable(uart, UART_RXD);
}

static int uart_uwp_irq_tx_complete(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);
    u32_t status;

    status = uwp_uart_status(uart);

    return (status & UART_TXF_EMPTY);
}

static int uart_uwp_irq_rx_ready(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);

    return malin_uart_rx_ready(uart);
}

static void uart_uwp_irq_err_enable(struct device *dev)
{
    /* Not yet used in zephyr */
}

static void uart_uwp_irq_err_disable(struct device *dev)
{
    /* Not yet used in zephyr */
}

static int uart_uwp_irq_is_pending(struct device *dev)
{
    volatile struct uwp_uart *uart = UART_STRUCT(dev);
    u32_t status;

    status = uwp_uart_status(uart);

    return (status & (UART_TXF_EMPTY | UART_RXF_FULL));
}

static int uart_uwp_irq_update(struct device *dev)
{
    return 1;
}

static void uart_uwp_irq_callback_set(struct device *dev,
                     uart_irq_callback_user_data_t cb,
                     void *user_data)
{
    struct uart_uwp_dev_data_t * const dev_data = DEV_DATA(dev);

    dev_data->cb = cb;
}

static void uart_uwp_isr(void *arg)
{
    struct device *dev = arg;
    struct uart_uwp_dev_data_t * const dev_data = DEV_DATA(dev);

    if (dev_data->cb) {
        dev_data->cb(dev);
    }

    /* Clear interrupts here */
}
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */


static int aon_uart_uwp_init(serial_t *obj)
{
    volatile struct uwp_uart *uart = UART_STRUCT(obj);
    uwp_aon_enable(BIT(AON_EB_UART));
    uwp_aon_reset(BIT(AON_RST_UART));

    uwp_uart_set_cdk(uart, DIV_ROUND(obj->sys_clk_freq, obj->baud_rate));
    uwp_uart_set_stop_bit_num(uart, 1);
    uwp_uart_set_byte_len(uart, 3);

    uwp_uart_init(uart);

#ifdef CONFIG_UART_INTERRUPT_DRIVEN
    uwp_aon_intc_set_irq_callback(AON_INT_UART, aon_uart_uwp_isr, dev);
    uwp_aon_irq_enable(AON_INT_UART);
    uart_uwp_irq_rx_enable(dev);
#endif /* CONFIG_UART_INTERRUPT_DRIVEN */

    return 0;
}
#endif

// TODO: pin configuration
void serial_init(serial_t *obj, PinName tx, PinName rx){
    uart_uwp_init(obj);
}

void uart0_irq(void){
#if 0    
	 volatile struct uwp_uart *uart = (volatile struct uwp_uart *)stdio_uart.base;
	 if(uwp_uart_rx_ready(uart)){
         mbed_error_printf("%c ", uart->rxd.rxd);
     }
     mbed_error_printf("\r\n");
#endif
    volatile struct uwp_uart *uart = (volatile struct uwp_uart *)stdio_uart.base;
    uwp_uart_int_disable(uart, UART_RXD);
    (irq_handler[0])(0, RxIrq);
    uwp_uart_int_enable(uart, BIT(UART_RXF_FULL));
}
void serial_irq_set(serial_t *obj, SerialIrq irq, uint32_t enable){
    volatile struct uwp_uart *uart = UART_STRUCT(obj);
    uwp_uart_int_enable(uart, BIT(UART_RXF_FULL));
    NVIC_SetVector(UART0_IRQn, uart0_irq);
    NVIC_SetPriority(UART0_IRQn, 0x1FUL);
    NVIC_EnableIRQ(UART0_IRQn);
}


int  serial_getc(serial_t *obj){
    unsigned char ch;
    if(!uart_uwp_poll_in(obj,&ch))
        return (int)ch;
    return -1;
}

void serial_putc(serial_t *obj, int c){
    volatile struct uwp_uart *uart = UART_STRUCT(obj);
	if (uwp_uart_tx_ready(uart)) {
		uwp_uart_write(uart, c);
		while (!uwp_uart_trans_over(uart));
	}
}

void serial_baud(serial_t *obj, int baudrate){
    obj->baud_rate = baudrate;
    serial_init(obj, NC, NC);
}

int  serial_readable(serial_t *obj){
    volatile struct uwp_uart *uart = UART_STRUCT(obj);
    return uwp_uart_rx_ready(uart);
}

int  serial_writable(serial_t *obj){
    volatile struct uwp_uart *uart = UART_STRUCT(obj);
    return uwp_uart_tx_ready(uart);
}

// TODO: achieve following function

void serial_break_set(serial_t *obj){
    return;
}

void serial_break_clear(serial_t *obj){
    return;
}

void serial_format(serial_t *obj, int data_bits, SerialParity parity, int stop_bits){
    return;
}

void serial_irq_handler(serial_t *obj, uart_irq_handler handler, uint32_t id){
    irq_handler[0] = handler;
    return;
}





