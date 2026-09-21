#ifndef _APP_H
#define _APP_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "command.h"
#include "port.h"

enum app_state {
    APP_STATE_BOOT=0,
    APP_STATE_LOAD_SETTINGS,
    APP_STATE_INIT,
    APP_STATE_SERVICE_TASKS,
};

extern void yield();
extern void APP_Initialize ( void );
extern void APP_Tasks( void );
extern void input_password(struct port *port);
extern void input_username(struct port *port);
#endif