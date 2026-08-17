#include "uart.h"
#include "port.h"

//struct port *uart_ports[6];

struct uart_data uart_settings[] = {
    UARTDEF(6),
    UARTDEF(3),
    UARTDEF(5),
    UARTDEF(4),
    UARTDEF(2),
    UARTDEF(1),
};

TaskHandle_t uart_tasks_handle;

void uart_init(struct uart_data *data) {
    pin_set(data->txled, 1);
    pin_set(data->rxled, 1);

    pin_mode(data->txled, PIN_OUTPUT);
    pin_mode(data->rxled, PIN_OUTPUT);
    pin_mode(data->shutdown, PIN_OUTPUT);
    pin_set(data->shutdown, 1);
    
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

static void UART_Tasks(void *pvParameters) {
    while (1) {
        for (int i = 0; i < MAX_PORTS; i++) {
            if (ports[i].type == PORT_SERIAL) {   
                struct uart_data *data = (struct uart_data *)(ports[i].port_data);
                if (cb_available(&(ports[i].write_buffer))) {
                    if (data->fn_free() > 0) {
                        uart_write_byte(data, cb_read(&(ports[i].write_buffer)));
                    }
                }
                if (data->fn_avail() > 0) {
                    if (cb_free(&(ports[i].read_buffer)) > 0) {
                        pin_mode(data->rxled, 0);
                        pin_set(data->rxled, 1);
                        uint8_t b;
                        data->fn_read(&b, 1);
                        cb_write(&(ports[i].read_buffer), b);
                    }
                }
            }
        }
    }
}


void uart_boot() {

    for (int i = 0; i < 6; i++) {
        struct port *p = add_port(PORT_SERIAL, &uart_settings[i]);
        uart_init(&uart_settings[i]);
 //       char hello[50];
//        sprintf(hello, "\r\n\r\nThis is port %d\r\n", p->no);
        //uart_settings[i].fn_write(hello, strlen(hello));
        port_printf(p, "\r\n\r\nThis is port %d\r\n", p->no);
    }

    (void) xTaskCreate(
           (TaskFunction_t) UART_Tasks,
           "UART_Tasks",
           1024,   
           NULL,
           1U ,
           &uart_tasks_handle);
}
