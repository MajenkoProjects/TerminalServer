#ifndef _APP_H
#define _APP_H

#define VERSION "1.0.0"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"


enum app_state {
    APP_STATE_BOOT=0,
    APP_STATE_LOAD_SETTINGS,
    APP_STATE_INIT,
    APP_STATE_SERVICE_TASKS,
};

extern void APP_Initialize ( void );
extern void APP_Tasks( void );

#endif