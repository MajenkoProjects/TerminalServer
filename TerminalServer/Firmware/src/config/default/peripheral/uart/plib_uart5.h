/*******************************************************************************
  UART5 PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_uart5.h

  Summary:
    UART5 PLIB Header File

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef PLIB_UART5_H
#define PLIB_UART5_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "device.h"
#include "plib_uart_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Interface
// *****************************************************************************
// *****************************************************************************

#define UART5_FrequencyGet()    (uint32_t)(80000000UL)

/****************************** UART5 API *********************************/

void UART5_Initialize( void );

bool UART5_SerialSetup( UART_SERIAL_SETUP *setup, uint32_t srcClkFreq );

UART_ERROR UART5_ErrorGet( void );

bool UART5_AutoBaudQuery( void );

void UART5_AutoBaudSet( bool enable );

size_t UART5_Write(uint8_t* pWrBuffer, const size_t size );

size_t UART5_WriteCountGet(void);

size_t UART5_WriteFreeBufferCountGet(void);

size_t UART5_WriteBufferSizeGet(void);

bool UART5_TransmitComplete(void);

bool UART5_WriteNotificationEnable(bool isEnabled, bool isPersistent);

void UART5_WriteThresholdSet(uint32_t nBytesThreshold);

void UART5_WriteCallbackRegister( UART_RING_BUFFER_CALLBACK callback, uintptr_t context);

size_t UART5_Read(uint8_t* pRdBuffer, const size_t size);

size_t UART5_ReadCountGet(void);

size_t UART5_ReadFreeBufferCountGet(void);

size_t UART5_ReadBufferSizeGet(void);

bool UART5_ReadNotificationEnable(bool isEnabled, bool isPersistent);

void UART5_ReadThresholdSet(uint32_t nBytesThreshold);

void UART5_ReadCallbackRegister( UART_RING_BUFFER_CALLBACK callback, uintptr_t context);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif // PLIB_UART5_H
