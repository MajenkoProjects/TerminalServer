#ifndef _TTYPE_H
#define _TTYPE_H

#include <stdint.h>

#define SPECIAL_KEY 0x8000

#define IS_SPECIAL(X) (((X) & SPECIAL_KEY) == SPECIAL_KEY)

enum special_key {
    KEY_RETURN = 0,
    KEY_BACKSPACE,
    KEY_DEL,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_PF1,
    KEY_PF2,
    KEY_PF3,
    KEY_PF4,
    KEY_DO,
    KEY_HELP,
    KEY_ENTER,
    NUM_KEYS    
};

struct ttype {
    uint16_t columns;
    uint16_t lines;
    const char *keys[NUM_KEYS];
    const char *clreol;
    const char *cleft;
    const char *cright;
    const char *inschar;
    const char *delchar;
};


extern const struct ttype dumb;
extern const struct ttype vt100;
extern const struct ttype vt102;
#endif