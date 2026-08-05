#ifndef _LEDS_H
#define _LEDS_H

extern void init_leds();
extern void led_on(int id);
extern void led_off(int id);
extern void led_num(uint8_t v);

#endif