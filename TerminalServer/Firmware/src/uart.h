#ifndef _UART_H
#define _UART_H
#include <string.h>
#include <stdint.h>

#include "definitions.h"
#include "pin.h"


#define UART_FLOW_NONE      0x00
#define UART_FLOW_CTSRTS    0x01
#define UART_FLOW_DTRDSR    0x02
#define UART_FLOW_XONXOFF   0x04

#define UART_STOP_1         0x01
#define UART_STOP_2         0x02

#define UART_PAR_NONE       0x00
#define UART_PAR_ODD        0x01
#define UART_PAR_EVEN       0x02

struct uart_data {
    uint32_t baud;
    uint8_t flow;
    uint8_t bits;
    uint8_t stop;
    uint8_t parity;
    struct pin *cts;
    struct pin *rts;
    struct pin *dtr;
    struct pin *dsr;
    struct pin *txled;
    struct pin *rxled;
    struct pin *status;
    struct pin *shutdown;
    void (*fn_init)();
    bool (*fn_setup)(UART_SERIAL_SETUP * serialSetup, uint32_t clkFrequency);
    size_t (*fn_read)(uint8_t *, const size_t);
    size_t (*fn_write)(uint8_t *, const size_t);
    size_t (*fn_avail)();
    size_t (*fn_free)();
};

#define UARTDEF(X) \
    { 9600, UART_FLOW_NONE, 8, UART_STOP_1, UART_PAR_NONE, \
        &pins[U##X##CTS], &pins[U##X##RTS], &pins[U##X##DTR], &pins[U##X##DSR], \
        &pins[U##X##TXLED], &pins[U##X##RXLED], &pins[U##X##STATUS], &pins[U##X##SHTDN], \
        &UART##X##_Initialize, &UART##X##_SerialSetup, \
        &UART##X##_Read, &UART##X##_Write, \
        &UART##X##_ReadCountGet, &UART##X##_WriteFreeBufferCountGet, }

extern void uart_config(struct uart_data *data);
extern void uart_boot();

#endif