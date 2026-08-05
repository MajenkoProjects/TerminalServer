
#include <proc/p32mx795f512l.h>

void init_leds() {
    /*
     * RD6  1
     * RD7  2
     * RF0  3
     * RF1  4
     * RA6  5
     * RA7  6
     * RE1  7
     * RE0  8
     * RG0  9
     * RG1  10
     * RG14 11
     * RG12 12
     */    
    TRISDbits.TRISD6 = 0;
    TRISDbits.TRISD7 = 0;
    TRISFbits.TRISF0 = 0;
    TRISFbits.TRISF1 = 0;
    TRISEbits.TRISE0 = 0;
    TRISEbits.TRISE1 = 0;
    TRISGbits.TRISG0 = 0;
    TRISGbits.TRISG1 = 0;
    TRISAbits.TRISA6 = 0;
    TRISAbits.TRISA7 = 0;
    TRISGbits.TRISG12 = 0;
    TRISGbits.TRISG14 = 0;

    LATDbits.LATD6 = 0;
    LATDbits.LATD7 = 0;
    LATFbits.LATF0 = 0;
    LATFbits.LATF1 = 0;
    LATEbits.LATE0 = 0;
    LATEbits.LATE1 = 0;
    LATGbits.LATG0 = 0;
    LATGbits.LATG1 = 0;
    LATAbits.LATA6 = 0;
    LATAbits.LATA7 = 0;
    LATGbits.LATG12 = 0;
    LATGbits.LATG14 = 0;
}

void led_set(int id, int val) {
    switch (id) {
        case 11:
            LATDbits.LATD6 = val; break;
        case 10:
            LATDbits.LATD7 = val; break;
            
        case 9:
            LATFbits.LATF0 = val; break;
        case 8:
            LATFbits.LATF1 = val; break;

        case 7:
            LATGbits.LATG1 = val; break;
        case 6:
            LATGbits.LATG0 = val; break;

        case 5:
            LATAbits.LATA6 = val; break;
        case 4:
            LATAbits.LATA7 = val; break;
            
            
            
            

        case 3:
            LATEbits.LATE0 = val; break;
        case 2:
            LATEbits.LATE1 = val; break;



        case 1:
            LATGbits.LATG14 = val; break;
        case 0:
            LATGbits.LATG12 = val; break;
        
    }
    
}

void led_off(int id) {
    led_set(id, 0);
}

void led_on(int id) {
    led_set(id, 1);
}

void led_num(uint8_t v) {
    led_set(0, v & 0x80 ? 1 : 0);
    led_set(1, v & 0x40 ? 1 : 0);
    led_set(2, v & 0x20 ? 1 : 0);
    led_set(3, v & 0x10 ? 1 : 0);
    led_set(4, v & 0x08 ? 1 : 0);
    led_set(5, v & 0x04 ? 1 : 0);
    led_set(6, v & 0x02 ? 1 : 0);
    led_set(7, v & 0x01 ? 1 : 0);
}