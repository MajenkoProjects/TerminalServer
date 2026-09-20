#include <string.h>

#include "app.h"
#include "usb.h"
#include "port.h"

struct port *usb_ports[USB_DEVICE_CDC_INSTANCES_NUMBER];
struct usb_port_data usb_data[USB_DEVICE_CDC_INSTANCES_NUMBER];

USB_DEVICE_HANDLE USBDeviceHandle;

volatile bool usb_is_configured = false;

enum usb_state usb_sm_state = USB_STATE_INIT;

TaskHandle_t usb_tasks_handle;

USB_DEVICE_CDC_EVENT_RESPONSE APP_USBDeviceCDCEventHandler(USB_DEVICE_CDC_INDEX index, USB_DEVICE_CDC_EVENT event, void* pData, uintptr_t userData) {

    USB_CDC_CONTROL_LINE_STATE * controlLineStateData;
    uint16_t * breakData;
    USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *eventDataRead;
    switch (event) {
        case USB_DEVICE_CDC_EVENT_GET_LINE_CODING:
            USB_DEVICE_ControlSend(USBDeviceHandle,
                    &usb_data[index].getLineCodingData,
                    sizeof(USB_CDC_LINE_CODING));
            break;

        case USB_DEVICE_CDC_EVENT_SET_LINE_CODING:
            USB_DEVICE_ControlReceive(USBDeviceHandle,
                    &usb_data[index].setLineCodingData,
                    sizeof(USB_CDC_LINE_CODING));
            break;

        case USB_DEVICE_CDC_EVENT_SET_CONTROL_LINE_STATE:
            controlLineStateData = (USB_CDC_CONTROL_LINE_STATE *)pData;
            usb_data[index].controlLineStateData.dtr = controlLineStateData->dtr;
            if (usb_data[index].controlLineStateData.carrier != controlLineStateData->carrier) {
                if (controlLineStateData->carrier == 1) {
                    usb_ports[index]->mode = MODE_GREET;
                }
            }
            usb_data[index].controlLineStateData.carrier = controlLineStateData->carrier;
            USB_DEVICE_ControlStatus(USBDeviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_SEND_BREAK:
            breakData = (uint16_t *)pData;
            usb_data[index].breakData = *breakData;
            USB_DEVICE_ControlStatus(USBDeviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_READ_COMPLETE:
            eventDataRead =  (USB_DEVICE_CDC_EVENT_DATA_READ_COMPLETE *)pData;
            if(eventDataRead->status != USB_DEVICE_CDC_RESULT_ERROR) {
                usb_data[index].read_data_length = eventDataRead->length;
            } else {
                usb_data[index].read_data_length = 0;                
            }
            usb_data[index].read_complete = true;
            usb_data[index].read_data_pos = 0;
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_RECEIVED:
            USB_DEVICE_ControlStatus(USBDeviceHandle, USB_DEVICE_CONTROL_STATUS_OK);
            break;

        case USB_DEVICE_CDC_EVENT_CONTROL_TRANSFER_DATA_SENT:
            break;

        case USB_DEVICE_CDC_EVENT_WRITE_COMPLETE:
            xSemaphoreTakeFromISR(usb_data[index].write_running, NULL);
            // If anything is in output CB then queue it here
            break;

        default:
            break;
    }
    return USB_DEVICE_CDC_EVENT_RESPONSE_NONE;
}


void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * pData, uintptr_t context)
{
    uint8_t configurationValue;

    switch( event )
    {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:

            usb_is_configured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:
            /*do we have access to USB, if not try again*/
            configurationValue = ((USB_DEVICE_EVENT_DATA_CONFIGURED *)pData)->configurationValue;
            if(configurationValue == 1)
            {
                //USBDeviceTask_State = USBDEVICETASK_PROCESSUSBEVENTS_STATE;
                for (int i = 0; i < USB_DEVICE_CDC_INSTANCES_NUMBER; i++) {
                    USB_DEVICE_CDC_EventHandlerSet(i, APP_USBDeviceCDCEventHandler, (uintptr_t)&usb_data[i]);
                }
                usb_is_configured = true;

            }

            break;

        case USB_DEVICE_EVENT_SUSPENDED:
            break;

       case USB_DEVICE_EVENT_POWER_DETECTED:
            /* VBUS has been detected. We can attach the device */
            USB_DEVICE_Attach (USBDeviceHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:
            /* VBUS is not available. We can detach the device */
            USB_DEVICE_Detach(USBDeviceHandle);
            usb_is_configured = false;
            break;

        /* These events are not used in this demo */
        case USB_DEVICE_EVENT_RESUMED:
            if(usb_is_configured == true)
            {
            }
            break;

        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}








//static void USB_Tasks(void *pvParameters) {


void usb_transfer_data(struct port *port) {
    struct usb_port_data *data = (struct usb_port_data *)port->port_data;
    if (uxSemaphoreGetCount(data->write_running) == 0) {
        int a = xStreamBufferBytesAvailable(port->write_buffer);
        if (a > USB_BUFFER_SIZE) a = USB_BUFFER_SIZE;
        if (a > 0) {
            xStreamBufferReceive(port->write_buffer, data->write_buffer, a, 1);
            xSemaphoreGive(data->write_running);
            USB_DEVICE_CDC_Write(data->cdcInstance,
                &data->writeTransferHandle, 
                data->write_buffer, 
                a, 
                USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
        }
    }
    if (data->read_complete == true) {
        if (data->read_data_pos >= data->read_data_length) {
            data->read_complete = false;
            data->read_data_pos = 0;
            USB_DEVICE_CDC_Read(data->cdcInstance,
                &data->readTransferHandle,
                data->read_buffer, USB_BUFFER_SIZE);                
        } else {
            int space = xStreamBufferSpacesAvailable(port->read_buffer);
            int avail = data->read_data_length - data->read_data_pos;
            if (avail > space) avail = space;
            data->read_data_pos += xStreamBufferSend(port->read_buffer, &data->read_buffer[data->read_data_pos], avail, 1);
        }
    }
}

void usb_flush(struct port *port) {
    while (xStreamBufferBytesAvailable(port->write_buffer) > 0) {
        usb_transfer_data(port);
    }
}

void usb_task() {

    if (!usb_is_configured) return;

    for (int i = 0; i < USB_DEVICE_CDC_INSTANCES_NUMBER; i++) {
        usb_transfer_data(usb_ports[i]);
    }
}

void usb_create_ports() {
    for (int i = 0; i < USB_DEVICE_CDC_INSTANCES_NUMBER; i++) {
        usb_ports[i] = add_port(PORT_CDC, &usb_data[i]);
        usb_ports[i]->fn_flush = &usb_flush;
        usb_ports[i]->fn_yield = &usb_transfer_data;
        usb_data[i].write_running = xSemaphoreCreateBinary();
        usb_data[i].read_complete = false;
        usb_data[i].cdcInstance = i;
    }    
}


void USB_Initialize() {
    USBDeviceHandle = USB_DEVICE_Open( USB_DEVICE_INDEX_0, DRV_IO_INTENT_READWRITE );

    if(USBDeviceHandle != USB_DEVICE_HANDLE_INVALID) {
        USB_DEVICE_EventHandlerSet(USBDeviceHandle, APP_USBDeviceEventHandler, 0);
    
        while (!usb_is_configured);

        for (int i = 0; i < USB_DEVICE_CDC_INSTANCES_NUMBER; i++) {
            USB_DEVICE_CDC_Read(i,
                    &usb_data[i].readTransferHandle,
                    usb_data[i].read_buffer, USB_BUFFER_SIZE);  
            usb_data[i].read_complete = false;
        }

        if (USB_DEVICE_CDC_INSTANCES_NUMBER != 1) {
            port_printf(CONSOLE, "%d USB CDC/ACM ports initialized\r\n", USB_DEVICE_CDC_INSTANCES_NUMBER);
        } else {
            port_printf(CONSOLE, "%d USB CDC/ACM port initialized\r\n", USB_DEVICE_CDC_INSTANCES_NUMBER);        
        }
    } else {
        port_printf(CONSOLE, "Error configuring USB\r\n");
    }
    CONSOLE->fn_flush(CONSOLE);
}

int count = 0;

void test_write() {
    count++;
    sprintf((char *)usb_data[0].write_buffer, "%d AH AH AH!\r\n", count);
    taskENTER_CRITICAL();
    if (USB_DEVICE_CDC_Write(
            0, &usb_data[0].writeTransferHandle, usb_data[0].write_buffer, strlen((char *)usb_data[0].write_buffer), USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE) == USB_DEVICE_CDC_RESULT_OK) {
        //usb_data[0].write_running = true;
    } else {
    }
    taskEXIT_CRITICAL();
}


void fail_write() {
    sprintf((char *)usb_data[0].write_buffer, "Failed\r\n");
    USB_DEVICE_CDC_Write(0, &usb_data[0].writeTransferHandle, usb_data[0].write_buffer, 8, USB_DEVICE_CDC_TRANSFER_FLAGS_DATA_COMPLETE);
    //usb_data[0].write_running = true;
}

void usb_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data) {
}

void usb_set_name(struct port *port, const char *name) {
    snprintf(port->name, 9, name);
    port->name[8] = 0;
}