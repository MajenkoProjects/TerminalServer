#include <string.h>
#include <fcntl.h>
#include <stdio.h>
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
#include "mdns.h"
#include "wifi.h"
#include "modem.h"
#include "tu58.h"


struct module {
    void (*fn_boot)();
    void (*fn_init)();
    void (*fn_task)();
};

void system_greeter();
void final_boot_message();

void spi_open();
void spi_tasks();

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
                            { NULL,                     &mdns_init,             &mdns_tasks },
    //                        { &wifi_open,               NULL,                   &wifi_tasks },
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
    static struct port *port;
    
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
                state = APP_STATE_INIT_PORTS;
                port = ports;
            }
            break;
            
        case APP_STATE_INIT_PORTS:
            if (port_access_functions[port->access].init != NULL) {
                port_access_functions[port->access].init(port);
            }
            port = port->next;            
            if (!port) {
                state = APP_STATE_SERVICE_TASKS;
                port = ports;
                break;
            }
            break;
            
        case APP_STATE_SERVICE_TASKS: 

            switch (port->mode) {
                // A port is sitting doing nothing. If it's a serial port
                // and in local access mode then respond to a RETURN
                // keypress to initiate a login prompt.
                case MODE_IDLE:
                    if (port->access == ACCESS_LOCAL) {
                        if (port->type == PORT_SERIAL) {
                            if (port_available(port)) {
                                int c = port_read_byte(port);
                                if (c == 13) {
                                    port_set_mode(port, MODE_GREET);
                                }
                            }
                        }
                    }
                    break;

                // A short delay before presenting the greeting and
                // login prompt. Really only for telnet-in.
                case MODE_PREGREET: 
                    if (port->ticks == 0) {
                        port->ticks = xTaskGetTickCount();
                    } else if (xTaskGetTickCount() - port->ticks > 500) {
                        port->ticks = 0;
                        port_set_mode(port, MODE_GREET);
                    }
                    break;

                // The main greeting and login prompt display.
                case MODE_GREET:
                    if (port->access == ACCESS_LOCAL) {
                        greet(port);
                    }

                    break;


                // Main Local> prompt processing mode. Deal with all commands
                // entered.
                case MODE_LOCAL:
                    if (port_access_functions[port->access].process) {
                        port_access_functions[port->access].process(port);
                    }
                    break;

                // Session mode - pass data from the parent to the target
                // and back from the target to the parent. Deal with
                // local switch keypresses.
                case MODE_SESSION:

                    if ((port->access == ACCESS_MODEM) && (port->misc[62] >= 3)) {
                        if ((xTaskGetTickCount() - port->ticks) > 500) {
                            modem_response(port, MODEM_OK);
                            port_set_mode(port, MODE_MODEM);
                            port->ticks = 0;
                            port->misc[62] = 0;
                        }
                    }

                    if (port->active_session && (port->active_session->type == SESSION_DIRECT)) {
                        if (port_available(port) && (xStreamBufferSpacesAvailable(port->active_session->target->write_buffer))) {
                            int c = port_read_byte(port);

                            if (port->access == ACCESS_MODEM) { // Handle +++ for break
                                if (c == '+') {
                                    port->misc[62]++;
                                    if (port->misc[62] >= 3) {
                                        port->ticks = xTaskGetTickCount();
                                    }
                                } else {
                                    port->misc[62] = 0;
                                    port->ticks = 0;
                                }
                            }

                            uint16_t tmp[11];
                            int r = fancy_read(port, c, tmp, 10);
                            for (int i = 0; i < r; i++) {
                                if (tmp[i] == port->local_switch) {
                                    port_printf(port, "+++ BREAK +++\r\n");
                                    port_set_mode(port, MODE_LOCAL);
                                } else if (tmp[i] == port->forward_switch) {
                                    struct session *first = NULL;
                                    struct session *curr = NULL;
                                    struct session *next = NULL;
                                    for (struct session *sess = sessions; sess; sess = sess->next) {
                                        if (sess->type == SESSION_DELETED) continue;
                                        if (sess->parent == port) {
                                            if (first == NULL) first = sess;
                                            if (sess == port->active_session) {
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
                                    port_set_active_session(port, next);
                                } else if (tmp[i] == port->backward_switch) {
                                    struct session *prev = NULL;
                                    struct session *curr = NULL;
                                    struct session *last = NULL;
                                    for (struct session *sess = sessions; sess; sess = sess->next) {
                                        if (sess->type == SESSION_DELETED) continue;
                                        if (sess->parent == port) {
                                            last = sess;
                                            if (sess == port->active_session) {
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
                                    port_set_active_session(port, prev);

                                } else {
                                    if (IS_SPECIAL(tmp[i])) {
                                        const char *key = port->tinfo->keys[tmp[i] & 0xFF];
                                        for (int j = 0; j < strlen(key); j++) {
                                            port_write_byte(port->active_session->target, key[j]);                                                    
                                        }
                                    } else {
                                        port_write_byte(port->active_session->target, tmp[i]);
                                    }
                                }
                            }
                        }
                        if (port_available(port->active_session->target) && xStreamBufferSpacesAvailable(port->write_buffer)) {
                            int c = port_read_byte(port->active_session->target);
                            port_write_byte(port, c);               
                        }
                    }
                    break;

                default:
                    break;
            }
            
            
            port = port->next;
            if (!port) port = ports;
            break;
       
        default:
            break;
    }

    for (int i = 0; i < NUM_MODULES; i++) {
        if (modules[i].fn_task) modules[i].fn_task();
    }
    
}


void EMERG_PUTSTR(const char *str) {
    while (*str) {
        while (U6STAbits.UTXBF);
        U6TXREG = *str;        
        str++;
    }
}

void __attribute__((noreturn)) _general_exception_handler ( void ) {
    uint32_t exception_code = ((_CP0_GET_CAUSE() & 0x0000007CU) >> 2U);
    uint32_t exception_address = _CP0_GET_EPC();
    
    char tmp[128];
    sprintf(tmp, "\r\n\r\nGuru Meditation %08x.%08x\r\n", exception_code, exception_address);
    EMERG_PUTSTR(tmp);

    __builtin_software_breakpoint();//    DBG("\r\n\r\nGuru Meditation %08x.%08x\r\n", exception_code, exception_address);
    while (1);
}