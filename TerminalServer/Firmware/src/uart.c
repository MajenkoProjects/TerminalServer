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

static uint8_t uart_parity(uint8_t in, enum parity parity) {
    uint8_t t = in;
    uint8_t p = 0;
    switch (parity) {
        case PARITY_NONE:
            return in;
        case PARITY_MARK:
            return in | 0x80;
        case PARITY_SPACE:
            return in & 0x7F;
        case PARITY_EVEN:
            for (int i = 0; i < 7; i++) {
                p ^= (t & 0x01);
                t>>=1;
            }
            return (in & 0x7F) | (p << 7);
        case PARITY_ODD:
            for (int i = 0; i < 7; i++) {
                p ^= (t & 0x01);
                t>>=1;
            }
            p ^= 1;
            return (in & 0x7F) | (p << 7);
            
    }
    return in;
}

static void uart_calculate_parity(struct port *port, uint8_t *buf, int len) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    if (data->bits == 8) return;
    for (int i = 0; i < len; i++) {
        buf[i] = uart_parity(buf[i], data->parity);
    }
}

// We don't really care about the parity. Just strip the parity bit out
// and throw it away. We *could* check it, but... meh...
static void uart_check_parity(struct port *port, uint8_t *buf, int len) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    if (data->bits == 8) return;
    for (int i = 0; i < len; i++) {
        buf[i] &= 0x7F;
    }
}


void uart_stop(struct port *port) {
    //port_printf(CONSOLE, "STOP\r\n");
    struct uart_data *data = (struct uart_data *)port->port_data;
    uint8_t c;
    switch (data->flow) {
        case FLOW_RTSCTS:
            GPIO_PinSet(data->rts);
            break;
        case FLOW_DTRDSR:
            GPIO_PinSet(data->dtr);
            break;
        case FLOW_XONXOFF:
            c = 19;
            uart_calculate_parity(port, &c, 1);
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
        case FLOW_RTSCTS:
            GPIO_PinClear(data->rts);
            break;
        case FLOW_DTRDSR:
            GPIO_PinClear(data->dtr);
            break;
        case FLOW_XONXOFF:
            c = 17;
            uart_calculate_parity(port, &c, 1);
            data->fn_write(&c, 1);
            break;
        default:
            break;
    }
}

bool uart_can_tx(struct port *port) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    switch (data->flow) {
        case FLOW_RTSCTS:
            return GPIO_PinRead(data->cts) == 0;
        case FLOW_DTRDSR:
            return GPIO_PinRead(data->dsr) == 0;
        case FLOW_XONXOFF:
            return !data->paused;
        default:
            break;
    }
    return true;
}

static void uart_init(struct port *port) {
    struct uart_data *data = port->port_data;
    GPIO_PinSet(data->txled);
    GPIO_PinSet(data->rxled);

    GPIO_PinSet(data->shutdown);
    
    GPIO_PinSet(data->dtr);
    GPIO_PinClear(data->rts);
    
    data->fn_init();
    uart_config(data);
    
    GPIO_PinClear(data->txled);
    GPIO_PinClear(data->rxled);
    
}

void uart_config(struct uart_data *data) {
    UART_SERIAL_SETUP setup;
    
    setup.baudRate = data->baud;
  
    switch (data->bits) {
        case 7:
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
    
    
    if (data->bits == 8) {
        switch (data->parity) {
            case PARITY_NONE:
                setup.parity = UART_PARITY_NONE;
                break;
            case PARITY_EVEN:
                setup.parity = UART_PARITY_EVEN;
                break;
            case PARITY_ODD:
                setup.parity = UART_PARITY_ODD;
                break;
            default:
                setup.parity = UART_PARITY_INVALID;
                break;
        }
    } else {
        // We do parity manually for 7 bit.
        setup.parity = UART_PARITY_NONE;        
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

//int uart_write_byte(struct port *port, uint8_t b) {
//    struct uart_data *data = (struct uart_data *)(port->port_data);
//    uart_calculate_parity(port, &b, 1);
//    data->fn_write(&b, 1);
//    return 1;
//}





//static void UART_Tasks(void *pvParameters) {

void uart_transfer_data(struct port *port) {
    
    int available_bytes;
    int free_bytes;
    uint8_t temp[CIRCULAR_BUFFER_SIZE];
    struct uart_data *data = (struct uart_data *)(port->port_data);
    uint32_t ts = xTaskGetTickCount();

    if (port->fn_can_tx(port)) {
        int available_bytes = xStreamBufferBytesAvailable(port->write_buffer);
        int free_bytes = data->fn_free();
        if (available_bytes > free_bytes) available_bytes = free_bytes;

        if (available_bytes > 0) {                        
            GPIO_PinSet(data->txled);
            data->txled_ts = ts;
            xStreamBufferReceive(port->write_buffer, temp, available_bytes, 1);
            uart_calculate_parity(port, temp, available_bytes);
            data->fn_write(temp, available_bytes);
        }
    }
                
    available_bytes = data->fn_avail();
    free_bytes = xStreamBufferSpacesAvailable(port->read_buffer);
    if (available_bytes > free_bytes) available_bytes = free_bytes;
    if (available_bytes > 0) {
        GPIO_PinSet(data->rxled);
        data->rxled_ts = ts;
        data->fn_read(temp, available_bytes);
        uart_check_parity(port, temp, available_bytes);
        if (((port->access == ACCESS_REMOTE) && in_session(port)) || (port->access == ACCESS_LOCAL) || (port->access == ACCESS_MODEM) || (port->access == ACCESS_TU58)) {
            for (int i = 0; i < available_bytes; i++) {
                uint8_t b = temp[i];
                if ((data->flow == FLOW_XONXOFF) && (b == 17)) {
                    data->paused = false;
                } else if ((data->flow == FLOW_XONXOFF) && (b == 19)) {
                    data->paused = true;
                } else {
                    int lev1 = xStreamBufferBytesAvailable(port->read_buffer);
                    xStreamBufferSend(port->read_buffer, &b, 1, 1);
                    int lev2 = xStreamBufferBytesAvailable(port->read_buffer);
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
      
        int water = xStreamBufferBytesAvailable(port->read_buffer);
        if (port->stopped && (port->waterlevel > port->low_water) && (water <= port->low_water)) {
            port->fn_start(port);
            port->stopped = false;
        }
        port->waterlevel = water;

        uint32_t ts = xTaskGetTickCount();
        if ((data->txled_ts > 0) && ((ts - data->txled_ts) > 25)) {
            data->txled_ts = 0;
            GPIO_PinClear(data->txled);
        }

        if ((data->rxled_ts > 0) && ((ts - data->rxled_ts) > 25)) {
            data->rxled_ts = 0;
            GPIO_PinClear(data->rxled);
        }
                
        UART_ERROR err = data->fn_get_error();
                

        if (err == UART_ERROR_FRAMING) { // Break
            if (port->breakmode == BREAK_LOCAL) {
                port_printf(port, "+++ BREAK +++\r\n");
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
            uart_calculate_parity(port, &zero, 1);
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
    while (xStreamBufferBytesAvailable(port->write_buffer)) {
        uart_transfer_data(port);
    }
    struct uart_data *data = (struct uart_data *)port->port_data;
    while (data->fn_write_get() > 0) {
        vTaskDelay(1);
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
        case SETTINGS_UART_BAUD:
            port_data->baud = *(uint32_t *)data;
            break;
        case SETTINGS_UART_FLOW:
            port_data->flow = data[0];
            break;
        case SETTINGS_UART_PARITY:
            port_data->parity = data[0];
            break;
        case SETTINGS_UART_BITS:
            port_data->bits = data[0];
            break;
        case SETTINGS_UART_STOPBITS:
            port_data->stop = data[0];
            break;
          
    }
}




int get_index_from_port(struct port *port) {
    for (int i = 0; i < 6; i++) {
        if (port->port_data == &uart_settings[i]) {
            return i;
        }
    }
    return -1;
}


static error_t uart_show_status(struct port *port, struct port *target) {
    struct uart_data *data = (struct uart_data *)port->port_data;
    port_printf(port, "DTR: %3s    DSR: %3s    RTS: %3s    CTS: %3s\r\n",
            GPIO_PinRead(data->dtr)?"On":"Off",
            GPIO_PinRead(data->dsr)?"On":"Off",
            GPIO_PinRead(data->rts)?"On":"Off",
            GPIO_PinRead(data->cts)?"On":"Off"
            
            );
    port_printf(port, "Status: %3s\r\n", GPIO_PinRead(data->status)?"On":"Off");
    return ERR_OK;
}
static error_t uart_set_parity(struct port *port, enum parity parity) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    struct uart_data *data = (struct uart_data *)port->port_data;
    data->parity = parity;
    uart_config(&uart_settings[idx]);    
    return ERR_OK;
}
static error_t uart_set_baud(struct port *port, uint32_t baud) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    uart_settings[idx].baud = baud;
    uart_config(&uart_settings[idx]);
    return ERR_OK;
}
static error_t uart_set_bits(struct port *port, uint8_t bits) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    struct uart_data *data = (struct uart_data *)port->port_data;
    data->bits = bits;
    uart_config(&uart_settings[idx]);    
    return ERR_OK;
}
static error_t uart_set_stop(struct port *port, uint8_t stop) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    struct uart_data *data = (struct uart_data *)port->port_data;
    if (stop < 1) return ERR_INVALID;
    if (stop > 2) return ERR_INVALID;
    data->stop = stop;
    uart_config(&uart_settings[idx]);
    return ERR_OK;
}
static error_t uart_set_flow(struct port *port, enum flow flow) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    struct uart_data *data = (struct uart_data *)port->port_data;
    data->flow = flow;
    uart_config(&uart_settings[idx]);
    return ERR_OK;
}
static error_t uart_define_parity(struct port *port, enum parity parity) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    uint8_t p = parity;
    setting_set(MODULE_UART, SETTINGS_UART_PARITY, idx, 1, (uint8_t *)&p);
    return ERR_OK;
}
static error_t uart_define_baud(struct port *port, uint32_t baud) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    setting_set(MODULE_UART, SETTINGS_UART_BAUD, idx, 4, (uint8_t *)&baud);
    return ERR_OK;
}
static error_t uart_define_bits(struct port *port, uint8_t bits) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    setting_set(MODULE_UART, SETTINGS_UART_BITS, idx, 1, (uint8_t *)&bits);
    return ERR_OK;
}
static error_t uart_define_stop(struct port *port, uint8_t stop) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    setting_set(MODULE_UART, SETTINGS_UART_STOPBITS, idx, 1, (uint8_t *)&stop);
    return ERR_OK;
}
static error_t uart_define_flow(struct port *port, enum flow flow) {
    int idx = get_index_from_port(port);
    if (idx > 5) return ERR_BADPORT;
    uint8_t f = flow;
    setting_set(MODULE_UART, SETTINGS_UART_FLOW, idx, 1, (uint8_t *)&f);
    return ERR_OK;
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
        port->fn_status = &uart_show_status;

        port->set.parity = &uart_set_parity;
        port->set.speed = &uart_set_baud;
        port->set.bits = &uart_set_bits;
        port->set.stop = &uart_set_stop;
        port->set.flow = &uart_set_flow;
        
        port->define.parity = &uart_define_parity;
        port->define.speed = &uart_define_baud;
        port->define.bits = &uart_define_bits;
        port->define.stop = &uart_define_stop;
        port->define.flow = &uart_define_flow;
        
        uart_ports[i] = port;
    }    
}
