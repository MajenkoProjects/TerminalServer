#ifndef _USB_H
#define _USB_H

#include <stdint.h>
#include "usb/usb_device.h"
#include "usb/usb_device_cdc.h"
#include <FreeRTOS.h>
#include <semphr.h>

#include "port.h"


#define USB_BUFFER_SIZE 64
enum usb_state {
    USB_STATE_INIT=0,
    USB_STATE_WAIT_FOR_CONFIGURATION,
    USB_STATE_CHECK_IF_CONFIGURED,
    USB_STATE_CHECK_FOR_READ_COMPLETE,
    USB_STATE_CHECK_FOR_WRITE_COMPLETE,
    USB_STATE_WAIT_FOR_WRITE_COMPLETE,
    USB_STATE_ERROR
};




struct usb_port_data {
    USB_DEVICE_CDC_INDEX cdcInstance;
    USB_CDC_LINE_CODING setLineCodingData;
    USB_CDC_LINE_CODING getLineCodingData;
    USB_CDC_CONTROL_LINE_STATE controlLineStateData;
    uint16_t breakData;
    USB_DEVICE_CDC_TRANSFER_HANDLE readTransferHandle;
    USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;
    volatile bool read_complete;
    uint32_t read_data_length;
    uint32_t read_data_pos;
    uint8_t read_buffer[USB_BUFFER_SIZE] USB_ALIGN;
    uint8_t write_buffer[USB_BUFFER_SIZE] USB_ALIGN;
    SemaphoreHandle_t write_running;
};


extern void USB_Initialize();
extern void usb_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);
extern void usb_create_ports();
extern void usb_task();
#endif