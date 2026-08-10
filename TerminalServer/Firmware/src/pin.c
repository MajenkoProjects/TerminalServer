#include "pin.h"
#include "app.h"

/*
    volatile uint32_t *tris;
    volatile uint32_t *port;
    volatile uint32_t *lat;
    uint8_t bit;
 */



#define PINDEF(P, B) { &TRIS##P, &PORT##P, &LAT##P, B }

struct pin pins[] = {
    PINDEF(D, 6),   // U1TXLED
    PINDEF(D, 7),  // U1RXLED
    PINDEF(D, 12),  // U1CTS
    PINDEF(D, 5),   // U1RTS
    PINDEF(D, 4),   // U1DTR
    PINDEF(D, 13),  // U1DSR
    PINDEF(D, 3),   // U1STATUS
    PINDEF(D, 2),   // U1SHTDN

    PINDEF(F, 0),   // U2TXLED
    PINDEF(F, 1),   // U2RXLED
    PINDEF(D, 9),   // U2CTS
    PINDEF(D, 1),   // U2RTS
    PINDEF(C, 14),  // U2DTR
    PINDEF(C, 13),  // U2DSR
    PINDEF(D, 8),   // U2STATUS
    PINDEF(F, 3),   // U2SHTDN
    
    PINDEF(E, 0),   // U3TXLED
    PINDEF(E, 1),   // U3RXLED
    PINDEF(C, 2),   // U3CTS
    PINDEF(E, 9),   // U3RTS
    PINDEF(E, 8),   // U3DTR
    PINDEF(C, 13),  // U3DSR
    PINDEF(C, 1),   // U3STATUS
    PINDEF(E, 7),   // U3SHTDN
    
    PINDEF(G, 1),   // U4TXLED
    PINDEF(G, 0),   // U4RXLED
    PINDEF(B, 8),   // U4CTS
    PINDEF(B, 11),  // U4RTS
    PINDEF(B, 10),  // U4DTR
    PINDEF(B, 9),   // U4DSR
    PINDEF(A, 10),  // U4STATUS
    PINDEF(A, 9),   // U4SHTDN
    
    PINDEF(A, 6),   // U5TXLED
    PINDEF(A, 7),   // U5RXLED
    PINDEF(B, 3),   // U5CTS
    PINDEF(B, 7),   // U5RTS
    PINDEF(B, 6),   // U5DTR
    PINDEF(B, 2),   // U5DSR
    PINDEF(B, 4),   // U5STATUS
    PINDEF(B, 5),   // U5SHTDN
    
    PINDEF(G, 14),  // U6TXLED
    PINDEF(G, 12),  // U6RXLED
    PINDEF(E, 3),   // U6CTS
    PINDEF(E, 6),   // U6RTS
    PINDEF(E, 5),   // U6DTR
    PINDEF(E, 4),   // U6DSR
    PINDEF(E, 2),   // U6STATUS
    PINDEF(G, 13),  // U6SHTDN
};

void pin_mode(struct pin *pin, uint8_t mode) {
    if (mode == PIN_INPUT) {
        *(pin->tris) |= (1 << pin->bit);
    } else {
        *(pin->tris) &= ~(1 << pin->bit);
    }
}

void pin_set(struct pin *pin, uint8_t val) {
    if (val == 0) {
        *(pin->lat) &= ~(1 << pin->bit);
    } else {
        *(pin->lat) |= (1 << pin->bit);
    }    
}

uint8_t pin_get(struct pin *pin) {
    if (*(pin->port) & (1 << pin->bit)) {
        return 1;
    }
    return 0;
}