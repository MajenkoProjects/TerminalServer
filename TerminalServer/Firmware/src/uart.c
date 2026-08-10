#include "uart.h"

struct uart_data uart_settings[] = {
    UARTDEF(6),
    UARTDEF(3),
    UARTDEF(5),
    UARTDEF(4),
    UARTDEF(2),
    UARTDEF(1),
};

void uart_init(struct uart_data *data) {
    pin_mode(data->txled, PIN_OUTPUT);
    pin_mode(data->rxled, PIN_OUTPUT);
    
    
    pin_set(data->txled, 1);
    vTaskDelay(100);
    pin_set(data->rxled, 1);
    vTaskDelay(1000);
    data->fn_init();
    uart_config(data);
    
    pin_set(data->txled, 0);
    vTaskDelay(100);
    pin_set(data->rxled, 0);
    vTaskDelay(1000);
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

void uart_boot() {
    uart_init(&uart_settings[0]);
    uart_init(&uart_settings[1]);
    uart_init(&uart_settings[2]);
    uart_init(&uart_settings[3]);
    uart_init(&uart_settings[4]);
    uart_init(&uart_settings[5]);
}