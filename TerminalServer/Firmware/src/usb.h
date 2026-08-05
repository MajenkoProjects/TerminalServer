#ifndef _USB_H
#define _USB_H

#include <stdint.h>
#include "usb/usb_device.h"
#include "usb/usb_device_cdc.h"


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
    uint8_t read_buffer[USB_BUFFER_SIZE] USB_ALIGN;
    uint8_t write_buffer[USB_BUFFER_SIZE] USB_ALIGN;
    volatile bool write_running;

};


extern void USB_Initialize();
#endif