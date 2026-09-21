#include <string.h>
#include <fcntl.h>
#include <stdio.h>
//#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "app.h"
#include "usb.h"
#include "port.h"
#include "task.h"
#include "uart.h"
#include "command.h"
#include "telnet_in.h"
#include "settings.h"
#include "session.h"
#include "network.h"
#include "telnet_out.h"
#include "util.h"
#include "tcp_in.h"
#include "version.h"
//#include "arp_private.h"


struct module {
    void (*fn_boot)();
    void (*fn_init)();
    void (*fn_task)();
};

void system_greeter();
void final_boot_message();

// These function pointers define the boot sequence. First all the
// functions on the left are executed in order, then the stored settings
// are loaded from the EEPROM chip, then the functions in the middle are
// executed in order. Finally the function on the right is repeatedly called
// each iteration of the main thread passing a uint32_t tick counter.
static const struct module modules[] = {
    //                        Stage 1 boot              Stage 2 boot            Tasks
    /* System */            { &system_init_defaults,    NULL,                   NULL }, 
    /* UARTs */             { &uart_create_ports,       &uart_boot,             &uart_task },
    /* Boot banner */       { NULL,                     &system_greeter,        NULL },
    /* USB */               { &usb_create_ports,        &USB_Initialize,        &usb_task },
    /* Ethernet */          { &ethernet_init_defaults,  &ethernet_boot,         NULL },
    /* Telnet In */         { NULL,                     &telnet_in_initialize,  &telnet_in_task },
    /* Telnet Out */        { NULL,                     &telnet_out_initialize, &telnet_out_task },
    /* TCP In */            { NULL,                     &tcp_in_init,           &tcp_in_task },
    /* Final boot */        { NULL,                     &final_boot_message,    NULL },
};

#define NUM_MODULES (sizeof(modules) / sizeof(struct module))

extern      ssize_t write(int fildes, const void *buf, size_t nbyte);
extern      int close(int fildes);

static enum app_state state = APP_STATE_BOOT;

uint32_t tick = 0;

void system_greeter() {
    port_printf(CONSOLE, "\x0c\n\nMajenko Technologies Terminal Server V" VERSION "\r\n");
    CONSOLE->fn_flush(CONSOLE);
    port_printf(CONSOLE, "(c) 2026 Majenko Technologies, All Rights Reserved\r\n");
    CONSOLE->fn_flush(CONSOLE);
    port_printf(CONSOLE, "\r\n\n\n");
    CONSOLE->fn_flush(CONSOLE);
}

void final_boot_message() {
    port_printf(CONSOLE, "\nSystem initialized. Press <RETURN> to activate console.\r\n\n");
    CONSOLE->fn_flush(CONSOLE);

}

void input_username(struct port *port) {
    
    char *username = strtok(port->commands[port->cmdno], " \t");
    if (!username) {        
        return;
    }
    if (strlen(username) == 0) {
        return;        
    }
    memset(port->username, 0, 9);
    int l = strlen(port->commands[port->cmdno]);
    if (l > 8) l = 8;
    memcpy(port->username, port->commands[port->cmdno], l);
    port->cstate = CMD_LOCAL;
}

void input_password(struct port *port) {
    if (strcmp(port->commands[port->cmdno], system_settings.password) == 0) {
        port->priv = true;
    } else {
        port_printf(port, "%%Error: Incorrect password.\r\n");
        port->priv = false;
    }
    port->cstate = CMD_LOCAL;
}

void APP_Initialize ( void ) {
}

void yield() {
    for (struct port *port = ports; port; port = port->next) {
        if (port->type != PORT_NONE) {
            if (port->fn_yield) {
                port->fn_yield(port);
            }
        }
    }
}

void APP_Tasks ( void ) {    
    static uint32_t reset_ts = 0;
    static bool reset_state = true;
    static int modno = 0;
    
    if (GPIO_PinRead(FRES_PIN) != reset_state) {
        reset_state = GPIO_PinRead(FRES_PIN);
        
        if (reset_state == false) {
            reset_ts = xTaskGetTickCount();
            port_printf(CONSOLE, "Keep holding RESET for 30 seconds to factory reset.\r\n");
        }
        vTaskDelay(50);
    } 
    
    if ((reset_state == false) && ((xTaskGetTickCount() - reset_ts) > 30000)) {
        port_printf(CONSOLE, "Erasing NVRAM and rebooting. Please wait.\r\n");
        settings_erase();
        SYS_RESET_SoftwareReset();
    }
    
    switch ( state ) {
        case APP_STATE_BOOT:   
            if (modules[modno].fn_boot) modules[modno].fn_boot();
            modno++;
            if (modno >= NUM_MODULES) {
                modno = 0;
                state = APP_STATE_LOAD_SETTINGS;
            }
            break;
            
        case APP_STATE_LOAD_SETTINGS:
            load_settings(); 
            state = APP_STATE_INIT;
            break;
            
        case APP_STATE_INIT:
            if (modules[modno].fn_init) modules[modno].fn_init();
            modno++;
            if (modno >= NUM_MODULES) {
                modno = 0;
                state = APP_STATE_SERVICE_TASKS;
            }
            break;
            
        case APP_STATE_SERVICE_TASKS: 

            for (struct port *scan = ports; scan; scan = scan->next) {
                if (scan->type != PORT_NONE) {
                    bool have_prompted = false;
                    if (scan->mode != scan->previous_mode) {
                        scan->previous_mode = scan->mode;
                        if (scan->mode == MODE_LOCAL) {
                            port_printf(scan, prompt(scan));
                            if (scan->fn_flush) scan->fn_flush(scan);
                            have_prompted = true;
                        }
                    }
                    
                    switch (scan->mode) {
                        
                        // A port is sitting doing nothing. If it's a serial port
                        // and in local access mode then respond to a RETURN
                        // keypress to initiate a login prompt.
                        case MODE_IDLE:
                            if (scan->access == ACCESS_LOCAL) {
                                if (scan->type == PORT_SERIAL) {
                                    if (port_available(scan)) {
                                        int c = port_read_byte(scan);
                                        if (c == 13) {
                                            port_set_mode(scan, MODE_GREET);
                                        }
                                    }
                                }
                            }
                            break;

                        // A short delay before presenting the greeting and
                        // login prompt. Really only for telnet-in.
                        case MODE_PREGREET: 
                            if (scan->ticks == 0) {
                                scan->ticks = xTaskGetTickCount();
                            } else if (xTaskGetTickCount() - scan->ticks > 500) {
                                scan->ticks = 0;
                                port_set_mode(scan, MODE_GREET);
                            }
                            break;

                        // The main greeting and login prompt display.
                        case MODE_GREET:
                            if (scan->access == ACCESS_LOCAL) {
                                greet(scan);
                            }
                            break;


                        // Main Local> prompt processing mode. Deal with all commands
                        // entered.
                        case MODE_LOCAL:
                            if (scan->access == ACCESS_LOCAL) {
                                if (port_available(scan)) {
                                    int c = port_read_byte(scan);
                                    if (command_process(scan, c) == 1) {
                                        if ((!have_prompted) && (scan->mode == MODE_LOCAL)) {
                                            port_printf(scan, prompt(scan));
                                            if (scan->fn_flush) scan->fn_flush(scan);
                                            have_prompted = true;
                                        }
                                    }
                                }
                            }
                            break;
                            
                        // Session mode - pass data from the parent to the target
                        // and back from the target to the parent. Deal with
                        // local switch keypresses.
                        case MODE_SESSION:
                            if (scan->active_session && (scan->active_session->type == SESSION_DIRECT)) {
                                if (port_available(scan) && (xStreamBufferSpacesAvailable(scan->active_session->target->write_buffer))) {
                                    int c = port_read_byte(scan);
                                    uint16_t tmp[11];
                                    int r = fancy_read(scan, c, tmp, 10);
                                    for (int i = 0; i < r; i++) {
                                        if (tmp[i] == scan->local_switch) {
                                            port_printf(scan, "+++ OUT OF CHEESE ERROR +++\r\n");
                                            port_set_mode(scan, MODE_LOCAL);
                                        } else if (tmp[i] == scan->forward_switch) {
                                            struct session *first = NULL;
                                            struct session *curr = NULL;
                                            struct session *next = NULL;
                                            for (struct session *sess = sessions; sess; sess = sess->next) {
                                                if (sess->type == SESSION_DELETED) continue;
                                                if (sess->parent == scan) {
                                                    if (first == NULL) first = sess;
                                                    if (sess == scan->active_session) {
                                                        curr = sess;
                                                        continue;
                                                    }
        
                                                    if ((next == NULL) && (curr != NULL)) {
                                                        next = sess;
                                                        continue;
                                                    }
                                                }
                                            }
                                            if (next == NULL) next = first;
                                            port_set_active_session(scan, next);
                                        } else if (tmp[i] == scan->backward_switch) {
                                            struct session *prev = NULL;
                                            struct session *curr = NULL;
                                            struct session *last = NULL;
                                            for (struct session *sess = sessions; sess; sess = sess->next) {
                                                if (sess->type == SESSION_DELETED) continue;
                                                if (sess->parent == scan) {
                                                    last = sess;
                                                    if (sess == scan->active_session) {
                                                        curr = sess;
                                                        continue;
                                                    }
                                                    if ((curr == NULL)) {
                                                        prev = sess;
                                                        continue;
                                                    }
                                                }
                                            }
                                            if (prev == NULL) prev = last;
                                            port_set_active_session(scan, prev);
                                            
                                        } else {
                                            if (IS_SPECIAL(tmp[i])) {
                                                const char *key = scan->tinfo->keys[tmp[i] & 0xFF];
                                                for (int j = 0; j < strlen(key); j++) {
                                                    port_write_byte(scan->active_session->target, key[j]);                                                    
                                                }
                                            } else {
                                                port_write_byte(scan->active_session->target, tmp[i]);
                                            }
                                        }
                                    }
                                }
                                if (port_available(scan->active_session->target) && xStreamBufferSpacesAvailable(scan->write_buffer)) {
                                    int c = port_read_byte(scan->active_session->target);
                                    port_write_byte(scan, c);               
                                }
                            }
                            break;
 
                        default:
                            break;
                    }
                }
            }
            
            break;
       
        default:
            break;
    }

    for (int i = 0; i < NUM_MODULES; i++) {
        if (modules[i].fn_task) modules[i].fn_task();
    }
}

