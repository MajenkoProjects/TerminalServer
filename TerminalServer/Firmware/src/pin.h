#ifndef _PIN_H
#define _PIN_H

#include <stdint.h>

#define PIN_OUTPUT  0
#define PIN_INPUT   1

struct pin {
    volatile uint32_t *tris;
    volatile uint32_t *port;
    volatile uint32_t *lat;
    uint8_t bit;
};

enum pinname {
    U1TXLED = 0,
    U1RXLED,
    U1CTS,
    U1RTS,
    U1DTR,
    U1DSR,
    U1STATUS,
    U1SHTDN,
    
    U2TXLED,
    U2RXLED,
    U2CTS,
    U2RTS,
    U2DTR,
    U2DSR,
    U2STATUS,
    U2SHTDN,
    
    U3TXLED,
    U3RXLED,
    U3CTS,
    U3RTS,
    U3DTR,
    U3DSR,
    U3STATUS,
    U3SHTDN,
    
    U4TXLED,
    U4RXLED,
    U4CTS,
    U4RTS,
    U4DTR,
    U4DSR,
    U4STATUS,
    U4SHTDN,
    
    U5TXLED,
    U5RXLED,
    U5CTS,
    U5RTS,
    U5DTR,
    U5DSR,
    U5STATUS,
    U5SHTDN,
    
    U6TXLED,
    U6RXLED,
    U6CTS,
    U6RTS,
    U6DTR,
    U6DSR,
    U6STATUS,
    U6SHTDN,
    
};

extern struct pin pins[];


extern void pin_mode(struct pin *pin, uint8_t mode);
extern void pin_set(struct pin *pin, uint8_t val);
extern uint8_t pin_get(struct pin *pin);
#endif