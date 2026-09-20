#include <stdbool.h>
#include "uart.h"
#include "port.h"
#include "settings.h"
#include "session.h"

//struct port *uart_ports[6];

struct uart_data uart_settings[] = {
    UARTDEF(6),
    UARTDEF(3),
    UARTDEF(5),
    UARTDEF(4),
    UARTDEF(2),
    UARTDEF(1),
};

const char *flow_names[] = {
    "None",
    "Rts/Cts",
    "Dtr/Dsr",
    "Xon/Xoff"
};

const char *parity_names[] = {
    "None",
    "Odd",
    "Even"
};

static bool running = false;

static struct port *uart_ports[6];


//TaskHandle_t uart_tasks_handle;

void uart_stop(struct port *port) {
    //port_printf(CONSOLE, "STOP\r\n");
    struct uart_data *data = (struct uart_data *)port->port_data;
    uint8_t c;
    switch (data->flow) {
        case UART_FLOW_RTSCTS:
            pin_set(data->rts, 1);
            break;
        case UART_FLOW_DTRDSR:
            pin_set(data->dtr, 1);
            break;
        case UART_FLOW_XONXOFF:
            c = 19;
            data->fn_write(&c, 1);
            break;
        default:
            break;
    }
}

void uart_start(struct port *port) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    uint8_t c;
    switch (data->flow) {
        case UART_FLOW_RTSCTS:
            pin_set(data->rts, 0);
            break;
        case UART_FLOW_DTRDSR:
            pin_set(data->dtr, 0);
            break;
        case UART_FLOW_XONXOFF:
            c = 17;
            data->fn_write(&c, 1);
            break;
        default:
            break;
    }
}

bool uart_can_tx(struct port *port) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    switch (data->flow) {
        case UART_FLOW_RTSCTS:
            return pin_get(data->cts) == 0;
        case UART_FLOW_DTRDSR:
            return pin_get(data->dsr) == 0;
        case UART_FLOW_XONXOFF:
            return !data->paused;
        default:
            break;
    }
    return true;
}

static void uart_init(struct port *port) {
    struct uart_data *data = port->port_data;
    pin_set(data->txled, 1);
    pin_set(data->rxled, 1);

    pin_mode(data->txled, PIN_OUTPUT);
    pin_mode(data->rxled, PIN_OUTPUT);
    pin_mode(data->shutdown, PIN_OUTPUT);
    pin_set(data->shutdown, 1);
    
    pin_mode(data->dtr, PIN_OUTPUT);
    pin_mode(data->rts, PIN_OUTPUT);
    pin_set(data->dtr, 1);
    pin_set(data->rts, 0);
    pin_mode(data->dsr, PIN_INPUT);
    pin_mode(data->cts, PIN_INPUT);
    
    data->fn_init();
    uart_config(data);
    
    pin_set(data->txled, 0);
    pin_set(data->rxled, 0);
    
}

void uart_config(struct uart_data *data) {
    UART_SERIAL_SETUP setup;
    
    setup.baudRate = data->baud;
  
    switch (data->bits) {
        case 8:
            setup.dataWidth = UART_DATA_8_BIT;
            break;
        case 9:
            setup.dataWidth = UART_DATA_9_BIT;
            break;
        default:
            setup.dataWidth = UART_DATA_INVALID;
            break;
    }
    
    switch (data->parity) {
        case UART_PAR_NONE:
            setup.parity = UART_PARITY_NONE;
            break;
        case UART_PAR_EVEN:
            setup.parity = UART_PARITY_EVEN;
            break;
        case UART_PAR_ODD:
            setup.parity = UART_PARITY_ODD;
            break;
        default:
            setup.parity = UART_PARITY_INVALID;
            break;
    }

    switch (data->stop) {
        case UART_STOP_1:
            setup.stopBits = UART_STOP_1_BIT;
            break;
        case UART_STOP_2:
            setup.stopBits = UART_STOP_2_BIT;
            break;
        default:
            setup.stopBits = UART_STOP_INVALID;
            break;
    }
    data->fn_setup(&setup, 0);
}

int uart_write_byte(struct uart_data *data, uint8_t b) {
    data->fn_write(&b, 1);
    return 1;
}

//static void UART_Tasks(void *pvParameters) {

void uart_transfer_data(struct port *port) {
    
    int available_bytes;
    int free_bytes;
    uint8_t temp[CIRCULAR_BUFFER_SIZE];
    struct uart_data *data = (struct uart_data *)(port->port_data);
    uint32_t ts = xTaskGetTickCount();

    if (port->fn_can_tx(port)) {
        int available_bytes = cb_available(&port->write_buffer);
        int free_bytes = data->fn_free();
        if (available_bytes > free_bytes) available_bytes = free_bytes;

        if (available_bytes > 0) {                        
            pin_set(data->txled, 1);
            data->txled_ts = ts;
            for (int i = 0; i < available_bytes; i++) {
                temp[i] = cb_read(&port->write_buffer);
            }
            data->fn_write(temp, available_bytes);
        }
    }
                
    available_bytes = data->fn_avail();
    free_bytes = cb_free(&port->read_buffer);
    if (available_bytes > free_bytes) available_bytes = free_bytes;
    if (available_bytes > 0) {
        pin_set(data->rxled, 1);
        data->rxled_ts = ts;
        data->fn_read(temp, available_bytes);

        if (((port->access == ACCESS_REMOTE) && in_session(port)) || (port->access == ACCESS_LOCAL)) {
            for (int i = 0; i < available_bytes; i++) {
                uint8_t b = temp[i];
                if ((data->flow == UART_FLOW_XONXOFF) && (b == 17)) {
                    data->paused = false;
                } else if ((data->flow == UART_FLOW_XONXOFF) && (b == 19)) {
                    data->paused = true;
                } else {
                    int lev1 = cb_available(&port->read_buffer);
                    cb_write(&(port->read_buffer), b);
                    int lev2 = cb_available(&port->read_buffer);
                    if ((!port->stopped) && (lev1 < port->high_water) && (lev2 >= port->high_water)) {
                        port->fn_stop(port);
                        port->stopped = true;
                    }
                }
            }
        }
    }
}

void uart_task() {

    
    if (!running) return;

    for (int portno = 0; portno < 6; portno++) {
        struct port *port = uart_ports[portno];
        struct uart_data *data = (struct uart_data *)(port->port_data);
      
        int water = cb_available(&port->read_buffer);
        if (port->stopped && (port->waterlevel > port->low_water) && (water <= port->low_water)) {
            port->fn_start(port);
            port->stopped = false;
        }
        port->waterlevel = water;

        uint32_t ts = xTaskGetTickCount();
        if ((data->txled_ts > 0) && ((ts - data->txled_ts) > 25)) {
            data->txled_ts = 0;
            pin_set(data->txled, 0);
        }

        if ((data->rxled_ts > 0) && ((ts - data->rxled_ts) > 25)) {
            data->rxled_ts = 0;
            pin_set(data->rxled, 0);
        }
                
        UART_ERROR err = data->fn_get_error();
                

        if (err == UART_ERROR_FRAMING) { // Break
            if (port->breakmode == BREAK_LOCAL) {
                port_printf(port, "+++ OUT OF CHEESE ERROR +++\r\n");
                port->mode = MODE_LOCAL;
            } else if (port->breakmode == BREAK_REMOTE) {
                if (port->active_session) {
                    port->active_session->target->send_break = true;
                }
            }
        }
                
                
        if (port->send_break) {
            port->send_break = false;
            while (data->fn_write_get() > 0);
            while (!data->fn_tx_complete());
            uint32_t b = data->baud;
            data->baud = data->baud / 2;
            uart_config(data);
            uint8_t zero = 0xff;
            data->fn_write(&zero, 1);
            while (data->fn_write_get() > 0);
            while (!data->fn_tx_complete());
            data->baud = b;
            uart_config(data);
        }
                
        uart_transfer_data(port);
    }
}

static void uart_flush(struct port *port) {
    while (cb_available(&port->write_buffer)) {
        uart_transfer_data(port);
    }
    struct uart_data *data = (struct uart_data *)port->port_data;
    while (data->fn_write_get() > 0) {
        vTaskDelay(1);
    }
}

void uart_create_ports() {
    for (int i = 0; i < 6; i++) {
        struct port *port = add_port(PORT_SERIAL, &uart_settings[i]);
        port->local_switch = LOCAL_SWITCH_NONE;
        port->fn_stop = &uart_stop;
        port->fn_start = &uart_start;
        port->fn_can_tx = &uart_can_tx;
        port->fn_show_detail = &uart_show_port_characteristics;
        port->fn_flush = &uart_flush;
        port->fn_yield = &uart_transfer_data;
        uart_ports[i] = port;
    }    
}

void uart_boot() {

    for (int i = 0; i < 6; i++) {
        uart_init(uart_ports[i]);            
        uart_ports[i]->fn_start(uart_ports[i]);
        uart_ports[i]->stopped = false;
    }
    running = true;

//    (void) xTaskCreate(
//           (TaskFunction_t) UART_Tasks,
//           "UART_Tasks",
//           1024,   
//           NULL,
//           1U ,
//           &uart_tasks_handle);
}


void uart_show_port_characteristics(struct port *port, struct port *target) {

    struct uart_data *data = target->port_data;

    port_printf(port, "   Char Size/Stop Bits:         %d/%d    Input Speed:               %6d\r\n",
        data->bits, data->stop, data->baud);
    port_printf(port, "   Flow Ctrl:            %10s    Output Speed:              %6d\r\n",
        flow_names[data->flow], data->baud);
    port_printf(port, "   Parity:                     %4s    Modem Control:               None\r\n",
            parity_names[data->parity]);

}



void uart_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data) {

    if (index > 5) return;
    
    struct uart_data *port_data = &uart_settings[index];
    struct port *port = NULL;
    
    for (struct port *scan = ports; scan; scan = scan->next) {
        if (scan->port_data == port_data) {
            port = scan;
            break;
        }
    }

    if (port == NULL) return;
    
    switch (parameter) {
        case SETTINGS_UART_NAME:
            if (length > 8) length = 8;
            memset(port->name, 0, 9);
            memcpy(port->name, data, length);
            break;
        case SETTINGS_UART_BAUD:
            port_data->baud = *(uint32_t *)data;
            break;
        case SETTINGS_UART_FLAGS:
            port_data->parity = ((*(uint32_t *)data) >> 24) & 0xFF;
            port_data->stop = ((*(uint32_t *)data) >> 16) & 0xFF;
            port_data->bits = ((*(uint32_t *)data) >> 8) & 0xFF;
            port_data->flow = (*(uint32_t *)data) & 0xFF;
            break;
          
    }
}

bool uart_get_setting_name(uint8_t module, uint8_t parameter, uint8_t index, char *buf, uint8_t *len) {
    switch (parameter) {
        case SETTINGS_UART_NAME:
            *len = snprintf(buf, *len, "uart.%d.name", index+1);
            return true;
        case SETTINGS_UART_BAUD:
            *len = snprintf(buf, *len, "uart.%d.baud", index+1);
            return true;
    }
    return false;
}

bool uart_render_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data, char *buf, uint8_t *len) {
    switch (parameter) {
        case SETTINGS_UART_NAME:
            memset(buf, 0, *len);
            if (length > *len - 1) {
                length = *len - 1;
            }
            memcpy(buf, data, length);
            *len = length;
            return true;
        case SETTINGS_UART_BAUD:
            *len = snprintf(buf, *len, "%u", *(uint32_t *)data);
            return true;
    }
    return false;    
}


int get_index_from_port(struct port *port) {
    for (int i = 0; i < 6; i++) {
        if (port->port_data == &uart_settings[i]) {
            return i;
        }
    }
    return -1;
}

void uart_set_baud(struct port *port, uint32_t baud) {
    int idx = get_index_from_port(port);
    if (idx > 5) return;
    uart_settings[idx].baud = baud;
    uart_config(&uart_settings[idx]);
}

void uart_define_baud(struct port *port, uint32_t baud) {
    int idx = get_index_from_port(port);
    if (idx > 5) return;
 //   uart_settings[idx].baud = baud;
 //   uart_config(&uart_settings[idx]);
    setting_set(MODULE_UART, SETTINGS_UART_BAUD, idx, 4, (uint8_t *)&baud);
}

void uart_set_name(struct port *port, const char *name) {
    snprintf(port->name, 9, name);
    port->name[8] = 0;
}

void uart_define_name(struct port *port, const char *name) {
   // snprintf(port->name, 9, name);
   // port->name[8] = 0;
    int idx = get_index_from_port(port);
    if (idx > 5) return;
    setting_set(MODULE_UART, SETTINGS_UART_NAME, idx, strlen(name), (uint8_t *)name);
}

void uart_set_flow(struct port *port, uint8_t flow) {
    int idx = get_index_from_port(port);
    if (idx > 5) return;
    struct uart_data *data = (struct uart_data *)port->port_data;
    data->flow = flow;
    uart_config(&uart_settings[idx]);
}

void uart_define_flow(struct port *port, uint8_t flow) {
    int idx = get_index_from_port(port);
    if (idx > 5) return;
    struct uart_data *data = (struct uart_data *)port->port_data;
    data->flow = flow;
    uart_config(&uart_settings[idx]);

    uint32_t flags = (
            (data->parity << 24) |
            (data->stop << 16) |
            (data->bits << 8) |
            data->flow
            );
    setting_set(MODULE_UART, SETTINGS_UART_FLAGS, idx, 4, (uint8_t *)&flags);
}

void uart_show_status(struct port *port, struct port *target) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    port_printf(port, "DTR: %3s    DSR: %3s    RTS: %3s    CTS: %3s\r\n",
            pin_get(data->dtr)?"On":"Off",
            pin_get(data->dsr)?"On":"Off",
            pin_get(data->rts)?"On":"Off",
            pin_get(data->cts)?"On":"Off"
            
            );
    port_printf(port, "Status: %3s\r\n", pin_get(data->status)?"On":"Off");
}

