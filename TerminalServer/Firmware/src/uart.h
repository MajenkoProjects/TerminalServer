#ifndef _UART_H
#define _UART_H
#include <string.h>
#include <stdint.h>

#include "definitions.h"
#include "port.h"

#define UART_STOP_1         0x01
#define UART_STOP_2         0x02

#define SETTINGS_UART_BAUD          0x02
#define SETTINGS_UART_FLOW          0x03
#define SETTINGS_UART_BITS          0x04
#define SETTINGS_UART_PARITY        0x05
#define SETTINGS_UART_STOPBITS      0x06

struct uart_data {
    uint32_t baud;
    uint8_t flow;
    uint8_t bits;
    uint8_t stop;
    enum parity parity;
    uint8_t tx;
    uint8_t rx;
    uint8_t cts;
    uint8_t rts;
    uint8_t dtr;
    uint8_t dsr;
    uint8_t txled;
    uint8_t rxled;
    uint8_t status;
    uint8_t shutdown;
    uint32_t txled_ts;
    uint32_t rxled_ts;
    void (*fn_init)();
    bool (*fn_setup)(UART_SERIAL_SETUP * serialSetup, uint32_t clkFrequency);
    size_t (*fn_read)(uint8_t *, const size_t);
    size_t (*fn_write)(uint8_t *, const size_t);
    size_t (*fn_avail)();
    size_t (*fn_free)();
    size_t (*fn_write_get)();
    UART_ERROR (*fn_get_error)();
    bool (*fn_tx_complete)();
    bool paused;
    volatile uint32_t *UMODE;
};

#define UARTDEF(X) \
    { 9600, FLOW_NONE, 8, UART_STOP_1, PARITY_NONE, \
        U##X##TX_PIN, U##X##RX_PIN, \
        U##X##CTS_PIN, U##X##RTS_PIN, U##X##DTR_PIN, U##X##DSR_PIN, \
        U##X##TXLED_PIN, U##X##RXLED_PIN, U##X##STATUS_PIN, U##X##SHTDN_PIN, \
        0, 0, \
        &UART##X##_Initialize, &UART##X##_SerialSetup, \
        &UART##X##_Read, &UART##X##_Write, \
        &UART##X##_ReadCountGet, &UART##X##_WriteFreeBufferCountGet, &UART##X##_WriteCountGet, \
        &UART##X##_ErrorGet, &UART##X##_TransmitComplete, \
        false, &U##X##MODE}

extern void uart_config(struct uart_data *data);
extern void uart_create_ports();
extern void uart_boot();
extern void uart_task();
extern void print_uart_info(struct port *port, struct port *target);
extern void uart_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);
extern void uart_show_port_characteristics(struct port *port, struct port *target);
extern void uart_send_break(struct port *port);


#endif