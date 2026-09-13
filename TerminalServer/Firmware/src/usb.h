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

    /* Application waits for device configuration*/
    USB_STATE_WAIT_FOR_CONFIGURATION,
    
    /* Application checks if the device is still configured*/
    USB_STATE_CHECK_IF_CONFIGURED,

   /* A character is received from host */
    USB_STATE_CHECK_FOR_READ_COMPLETE,

    /* Wait for the transmit to get completed */ 
    USB_STATE_CHECK_FOR_WRITE_COMPLETE,

    /* Wait for the write to complete */
    USB_STATE_WAIT_FOR_WRITE_COMPLETE,

    /* Application Error state*/
    USB_STATE_ERROR

};




struct usb_port_data {
    /* CDC instance number */
    USB_DEVICE_CDC_INDEX cdcInstance;

    /* Set Line Coding Data */
    USB_CDC_LINE_CODING setLineCodingData;

    /* Get Line Coding Data */
    USB_CDC_LINE_CODING getLineCodingData;

    /* Control Line State */
    USB_CDC_CONTROL_LINE_STATE controlLineStateData;

    /* Break data */
    uint16_t breakData;

    /* Read transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE readTransferHandle;

    /* Write transfer handle */
    USB_DEVICE_CDC_TRANSFER_HANDLE writeTransferHandle;

    /* True if a character was read */
    volatile bool read_complete;

    /* This variable saves number of bytes of data received from the Host.
     * Application uses this variable to send back same amount of data to Host.*/
    uint32_t read_data_length;
    uint32_t read_data_pos;
    uint8_t read_buffer[USB_BUFFER_SIZE] USB_ALIGN;
    uint8_t write_buffer[USB_BUFFER_SIZE] USB_ALIGN;
    SemaphoreHandle_t write_running;

};


extern void USB_Initialize();
extern void usb_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);

extern void usb_set_name(struct port *port, const char *name);

extern void usb_create_ports();
#endif