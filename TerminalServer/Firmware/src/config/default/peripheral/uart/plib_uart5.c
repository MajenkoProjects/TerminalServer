/*******************************************************************************
  UART5 PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_uart5.c

  Summary:
    UART5 PLIB Implementation File

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

#include "device.h"
#include "plib_uart5.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: UART5 Implementation
// *****************************************************************************
// *****************************************************************************

static volatile UART_RING_BUFFER_OBJECT uart5Obj;

#define UART5_READ_BUFFER_SIZE      (128U)
#define UART5_READ_BUFFER_SIZE_9BIT (128U >> 1)
#define UART5_RX_INT_DISABLE()      IEC2CLR = _IEC2_U5RXIE_MASK;
#define UART5_RX_INT_ENABLE()       IEC2SET = _IEC2_U5RXIE_MASK;

static volatile uint8_t UART5_ReadBuffer[UART5_READ_BUFFER_SIZE];

#define UART5_WRITE_BUFFER_SIZE      (128U)
#define UART5_WRITE_BUFFER_SIZE_9BIT (128U >> 1)
#define UART5_TX_INT_DISABLE()       IEC2CLR = _IEC2_U5TXIE_MASK;
#define UART5_TX_INT_ENABLE()        IEC2SET = _IEC2_U5TXIE_MASK;

static volatile uint8_t UART5_WriteBuffer[UART5_WRITE_BUFFER_SIZE];

#define UART5_IS_9BIT_MODE_ENABLED()    ( (U5MODE) & (_U5MODE_PDSEL0_MASK | _U5MODE_PDSEL1_MASK)) == (_U5MODE_PDSEL0_MASK | _U5MODE_PDSEL1_MASK) ? true:false

static void UART5_ErrorClear( void )
{
    UART_ERROR errors = UART_ERROR_NONE;
    uint8_t dummyData = 0u;

    errors = (UART_ERROR)(U5STA & (_U5STA_OERR_MASK | _U5STA_FERR_MASK | _U5STA_PERR_MASK));

    if(errors != UART_ERROR_NONE)
    {
        /* If it's a overrun error then clear it to flush FIFO */
        if((U5STA & _U5STA_OERR_MASK) != 0U)
        {
            U5STACLR = _U5STA_OERR_MASK;
        }

        /* Read existing error bytes from FIFO to clear parity and framing error flags */
        while((U5STA & _U5STA_URXDA_MASK) != 0U)
        {
            dummyData = (uint8_t)U5RXREG;
        }

        /* Clear error interrupt flag */
        IFS2CLR = _IFS2_U5EIF_MASK;

        /* Clear up the receive interrupt flag so that RX interrupt is not
         * triggered for error bytes */
        IFS2CLR = _IFS2_U5RXIF_MASK;

    }

    // Ignore the warning
    (void)dummyData;
}

void UART5_Initialize( void )
{
    /* Set up UxMODE bits */
    /* STSEL  = 0 */
    /* PDSEL = 0 */

    U5MODE = 0x8;

    /* Enable UART5 Receiver and Transmitter */
    U5STASET = (_U5STA_UTXEN_MASK | _U5STA_URXEN_MASK | _U5STA_UTXISEL1_MASK );

    /* BAUD Rate register Setup */
    U5BRG = 173;

    /* Disable Interrupts */
    IEC2CLR = _IEC2_U5EIE_MASK;

    IEC2CLR = _IEC2_U5RXIE_MASK;

    IEC2CLR = _IEC2_U5TXIE_MASK;

    /* Initialize instance object */
    uart5Obj.rdCallback = NULL;
    uart5Obj.rdInIndex = 0;
    uart5Obj.rdOutIndex = 0;
    uart5Obj.isRdNotificationEnabled = false;
    uart5Obj.isRdNotifyPersistently = false;
    uart5Obj.rdThreshold = 0;

    uart5Obj.wrCallback = NULL;
    uart5Obj.wrInIndex = 0;
    uart5Obj.wrOutIndex = 0;
    uart5Obj.isWrNotificationEnabled = false;
    uart5Obj.isWrNotifyPersistently = false;
    uart5Obj.wrThreshold = 0;

    uart5Obj.errors = UART_ERROR_NONE;

    if (UART5_IS_9BIT_MODE_ENABLED())
    {
        uart5Obj.rdBufferSize = UART5_READ_BUFFER_SIZE_9BIT;
        uart5Obj.wrBufferSize = UART5_WRITE_BUFFER_SIZE_9BIT;
    }
    else
    {
        uart5Obj.rdBufferSize = UART5_READ_BUFFER_SIZE;
        uart5Obj.wrBufferSize = UART5_WRITE_BUFFER_SIZE;
    }


    /* Turn ON UART5 */
    U5MODESET = _U5MODE_ON_MASK;

    /* Enable UART5_FAULT Interrupt */
    IEC2SET = _IEC2_U5EIE_MASK;

    /* Enable UART5_RX Interrupt */
    IEC2SET = _IEC2_U5RXIE_MASK;
}

bool UART5_SerialSetup( UART_SERIAL_SETUP *setup, uint32_t srcClkFreq )
{
    bool status = false;
    uint32_t baud;
    uint32_t status_ctrl;
    uint32_t uxbrg = 0;

    if (setup != NULL)
    {
        baud = setup->baudRate;

        if ((baud == 0U) || ((setup->dataWidth == UART_DATA_9_BIT) && (setup->parity != UART_PARITY_NONE)))
        {
            return status;
        }

        if(srcClkFreq == 0U)
        {
            srcClkFreq = UART5_FrequencyGet();
        }

        /* Calculate BRG value */
        uxbrg = (((srcClkFreq >> 2) + (baud >> 1)) / baud);

        /* Check if the baud value can be set with low baud settings */
        if (uxbrg < 1U)
        {
            return status;
        }

        uxbrg -= 1U;

        if (uxbrg > UINT16_MAX)
        {
            return status;
        }

        /* Turn OFF UART5. Save UTXEN, URXEN and UTXBRK bits as these are cleared upon disabling UART */

        status_ctrl = U5STA & (_U5STA_UTXEN_MASK | _U5STA_URXEN_MASK | _U5STA_UTXBRK_MASK);

        U5MODECLR = _U5MODE_ON_MASK;

        if(setup->dataWidth == UART_DATA_9_BIT)
        {
            /* Configure UART5 mode */
            U5MODE = (U5MODE & (~_U5MODE_PDSEL_MASK)) | setup->dataWidth;
        }
        else
        {
            /* Configure UART5 mode */
            U5MODE = (U5MODE & (~_U5MODE_PDSEL_MASK)) | setup->parity;
        }

        /* Configure UART5 mode */
        U5MODE = (U5MODE & (~_U5MODE_STSEL_MASK)) | setup->stopBits;

        /* Configure UART5 Baud Rate */
        U5BRG = uxbrg;

        if (UART5_IS_9BIT_MODE_ENABLED())
        {
            uart5Obj.rdBufferSize = UART5_READ_BUFFER_SIZE_9BIT;
            uart5Obj.wrBufferSize = UART5_WRITE_BUFFER_SIZE_9BIT;
        }
        else
        {
            uart5Obj.rdBufferSize = UART5_READ_BUFFER_SIZE;
            uart5Obj.wrBufferSize = UART5_WRITE_BUFFER_SIZE;
        }

        U5MODESET = _U5MODE_ON_MASK;

        /* Restore UTXEN, URXEN and UTXBRK bits. */
        U5STASET = status_ctrl;

        status = true;
    }

    return status;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static inline bool UART5_RxPushByte(uint16_t rdByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;
    uint32_t rdInIdx;

    tempInIndex = uart5Obj.rdInIndex + 1U;

    if (tempInIndex >= uart5Obj.rdBufferSize)
    {
        tempInIndex = 0U;
    }

    if (tempInIndex == uart5Obj.rdOutIndex)
    {
        /* Queue is full - Report it to the application. Application gets a chance to free up space by reading data out from the RX ring buffer */
        if(uart5Obj.rdCallback != NULL)
        {
            uintptr_t rdContext = uart5Obj.rdContext;

            uart5Obj.rdCallback(UART_EVENT_READ_BUFFER_FULL, rdContext);

            /* Read the indices again in case application has freed up space in RX ring buffer */
            tempInIndex = uart5Obj.rdInIndex + 1U;

            if (tempInIndex >= uart5Obj.rdBufferSize)
            {
                tempInIndex = 0U;
            }
        }
    }

    /* Attempt to push the data into the ring buffer */
    if (tempInIndex != uart5Obj.rdOutIndex)
    {
        uint32_t rdInIndex = uart5Obj.rdInIndex;

        if (UART5_IS_9BIT_MODE_ENABLED())
        {
            rdInIdx = uart5Obj.rdInIndex << 1U;
            UART5_ReadBuffer[rdInIdx] = (uint8_t)rdByte;
            UART5_ReadBuffer[rdInIdx + 1U] = (uint8_t)(rdByte >> 8U);
        }
        else
        {
            UART5_ReadBuffer[rdInIndex] = (uint8_t)rdByte;
        }

        uart5Obj.rdInIndex = tempInIndex;

        isSuccess = true;
    }
    else
    {
        /* Queue is full. Data will be lost. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void UART5_ReadNotificationSend(void)
{
    uint32_t nUnreadBytesAvailable;

    if (uart5Obj.isRdNotificationEnabled == true)
    {
        nUnreadBytesAvailable = UART5_ReadCountGet();

        if(uart5Obj.rdCallback != NULL)
        {
            uintptr_t rdContext = uart5Obj.rdContext;

            if (uart5Obj.isRdNotifyPersistently == true)
            {
                if (nUnreadBytesAvailable >= uart5Obj.rdThreshold)
                {
                    uart5Obj.rdCallback(UART_EVENT_READ_THRESHOLD_REACHED, rdContext);
                }
            }
            else
            {
                if (nUnreadBytesAvailable == uart5Obj.rdThreshold)
                {
                    uart5Obj.rdCallback(UART_EVENT_READ_THRESHOLD_REACHED, rdContext);
                }
            }
        }
    }
}

size_t UART5_Read(uint8_t* pRdBuffer, const size_t size)
{
    size_t nBytesRead = 0;
    uint32_t rdOutIndex = 0;
    uint32_t rdInIndex = 0;
    uint32_t rdOut16Idx;
    uint32_t nBytesRead16Idx;

    /* Take a snapshot of indices to avoid creation of critical section */
    rdOutIndex = uart5Obj.rdOutIndex;
    rdInIndex = uart5Obj.rdInIndex;

    while (nBytesRead < size)
    {
        if (rdOutIndex != rdInIndex)
        {
            if (UART5_IS_9BIT_MODE_ENABLED())
            {
                rdOut16Idx = rdOutIndex << 1U;
                nBytesRead16Idx = nBytesRead << 1U;

                pRdBuffer[nBytesRead16Idx] = UART5_ReadBuffer[rdOut16Idx];
                pRdBuffer[nBytesRead16Idx + 1U] = UART5_ReadBuffer[rdOut16Idx + 1U];
            }
            else
            {
                pRdBuffer[nBytesRead] = UART5_ReadBuffer[rdOutIndex];
            }
            nBytesRead++;
            rdOutIndex++;

            if (rdOutIndex >= uart5Obj.rdBufferSize)
            {
                rdOutIndex = 0U;
            }
        }
        else
        {
            /* No more data available in the RX buffer */
            break;
        }
    }

    uart5Obj.rdOutIndex = rdOutIndex;

    return nBytesRead;
}

size_t UART5_ReadCountGet(void)
{
    size_t nUnreadBytesAvailable;
    uint32_t rdInIndex;
    uint32_t rdOutIndex;

    /* Take a snapshot of indices to avoid processing in critical section */
    rdInIndex = uart5Obj.rdInIndex;
    rdOutIndex = uart5Obj.rdOutIndex;

    if ( rdInIndex >=  rdOutIndex)
    {
        nUnreadBytesAvailable =  rdInIndex -  rdOutIndex;
    }
    else
    {
        nUnreadBytesAvailable =  (uart5Obj.rdBufferSize -  rdOutIndex) + rdInIndex;
    }

    return nUnreadBytesAvailable;
}

size_t UART5_ReadFreeBufferCountGet(void)
{
    return (uart5Obj.rdBufferSize - 1U) - UART5_ReadCountGet();
}

size_t UART5_ReadBufferSizeGet(void)
{
    return (uart5Obj.rdBufferSize - 1U);
}

bool UART5_ReadNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = uart5Obj.isRdNotificationEnabled;

    uart5Obj.isRdNotificationEnabled = isEnabled;

    uart5Obj.isRdNotifyPersistently = isPersistent;

    return previousStatus;
}

void UART5_ReadThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        uart5Obj.rdThreshold = nBytesThreshold;
    }
}

void UART5_ReadCallbackRegister( UART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    uart5Obj.rdCallback = callback;

    uart5Obj.rdContext = context;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static bool UART5_TxPullByte(void* pWrData)
{
    bool isSuccess = false;
    uint32_t wrOutIndex = uart5Obj.wrOutIndex;
    uint32_t wrInIndex = uart5Obj.wrInIndex;
    uint32_t wrOut16Idx;
    uint8_t* pWrByte = (uint8_t*)pWrData;

    if (wrOutIndex != wrInIndex)
    {
        if (UART5_IS_9BIT_MODE_ENABLED())
        {
            wrOut16Idx = wrOutIndex << 1U;
            pWrByte[0] = UART5_WriteBuffer[wrOut16Idx];
            pWrByte[1] = UART5_WriteBuffer[wrOut16Idx + 1U];
        }
        else
        {
            *pWrByte = UART5_WriteBuffer[wrOutIndex];
        }
        wrOutIndex++;

        if (wrOutIndex >= uart5Obj.wrBufferSize)
        {
            wrOutIndex = 0U;
        }

        uart5Obj.wrOutIndex = wrOutIndex;

        isSuccess = true;
    }

    return isSuccess;
}

static inline bool UART5_TxPushByte(uint16_t wrByte)
{
    uint32_t tempInIndex;
    bool isSuccess = false;
    uint32_t wrOutIndex = uart5Obj.wrOutIndex;
    uint32_t wrInIndex = uart5Obj.wrInIndex;
    uint32_t wrIn16Idx;

    tempInIndex = wrInIndex + 1U;

    if (tempInIndex >= uart5Obj.wrBufferSize)
    {
        tempInIndex = 0U;
    }
    if (tempInIndex != wrOutIndex)
    {
        if (UART5_IS_9BIT_MODE_ENABLED())
        {
            wrIn16Idx = wrInIndex << 1U;
            UART5_WriteBuffer[wrIn16Idx] = (uint8_t)wrByte;
            UART5_WriteBuffer[wrIn16Idx + 1U] = (uint8_t)(wrByte >> 8U);
        }
        else
        {
            UART5_WriteBuffer[wrInIndex] = (uint8_t)wrByte;
        }

        uart5Obj.wrInIndex = tempInIndex;

        isSuccess = true;
    }
    else
    {
        /* Queue is full. Report Error. */
    }

    return isSuccess;
}

/* This routine is only called from ISR. Hence do not disable/enable USART interrupts. */
static void UART5_WriteNotificationSend(void)
{
    uint32_t nFreeWrBufferCount;

    if (uart5Obj.isWrNotificationEnabled == true)
    {
        nFreeWrBufferCount = UART5_WriteFreeBufferCountGet();

        if(uart5Obj.wrCallback != NULL)
        {
            uintptr_t wrContext = uart5Obj.wrContext;

            if (uart5Obj.isWrNotifyPersistently == true)
            {
                if (nFreeWrBufferCount >= uart5Obj.wrThreshold)
                {
                    uart5Obj.wrCallback(UART_EVENT_WRITE_THRESHOLD_REACHED, wrContext);
                }
            }
            else
            {
                if (nFreeWrBufferCount == uart5Obj.wrThreshold)
                {
                    uart5Obj.wrCallback(UART_EVENT_WRITE_THRESHOLD_REACHED, wrContext);
                }
            }
        }
    }
}

static size_t UART5_WritePendingBytesGet(void)
{
    size_t nPendingTxBytes;

    /* Take a snapshot of indices to avoid processing in critical section */

    uint32_t wrOutIndex = uart5Obj.wrOutIndex;
    uint32_t wrInIndex = uart5Obj.wrInIndex;

    if ( wrInIndex >=  wrOutIndex)
    {
        nPendingTxBytes =  wrInIndex - wrOutIndex;
    }
    else
    {
        nPendingTxBytes =  (uart5Obj.wrBufferSize -  wrOutIndex) + wrInIndex;
    }

    return nPendingTxBytes;
}

size_t UART5_WriteCountGet(void)
{
    size_t nPendingTxBytes;

    nPendingTxBytes = UART5_WritePendingBytesGet();

    return nPendingTxBytes;
}

size_t UART5_Write(uint8_t* pWrBuffer, const size_t size )
{
    size_t nBytesWritten  = 0;
    uint16_t halfWordData = 0U;

    while (nBytesWritten < size)
    {
        if (UART5_IS_9BIT_MODE_ENABLED())
        {
            halfWordData = pWrBuffer[(2U * nBytesWritten) + 1U];
            halfWordData <<= 8U;
            halfWordData |= pWrBuffer[(2U * nBytesWritten)];
            if (UART5_TxPushByte(halfWordData) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }
        else
        {
            if (UART5_TxPushByte(pWrBuffer[nBytesWritten]) == true)
            {
                nBytesWritten++;
            }
            else
            {
                /* Queue is full, exit the loop */
                break;
            }
        }

    }

    /* Check if any data is pending for transmission */
    if (UART5_WritePendingBytesGet() > 0U)
    {
        /* Enable TX interrupt as data is pending for transmission */
        UART5_TX_INT_ENABLE();
    }

    return nBytesWritten;
}

size_t UART5_WriteFreeBufferCountGet(void)
{
    return (uart5Obj.wrBufferSize - 1U) - UART5_WriteCountGet();
}

size_t UART5_WriteBufferSizeGet(void)
{
    return (uart5Obj.wrBufferSize - 1U);
}

bool UART5_TransmitComplete( void )
{
    bool transmitcompltecheck = false;
    if((U5STA & _U5STA_TRMT_MASK) != 0U)
    {
        transmitcompltecheck = true;
    }
    return transmitcompltecheck;
}

bool UART5_WriteNotificationEnable(bool isEnabled, bool isPersistent)
{
    bool previousStatus = uart5Obj.isWrNotificationEnabled;

    uart5Obj.isWrNotificationEnabled = isEnabled;

    uart5Obj.isWrNotifyPersistently = isPersistent;

    return previousStatus;
}

void UART5_WriteThresholdSet(uint32_t nBytesThreshold)
{
    if (nBytesThreshold > 0U)
    {
        uart5Obj.wrThreshold = nBytesThreshold;
    }
}

void UART5_WriteCallbackRegister( UART_RING_BUFFER_CALLBACK callback, uintptr_t context)
{
    uart5Obj.wrCallback = callback;

    uart5Obj.wrContext = context;
}

UART_ERROR UART5_ErrorGet( void )
{
    UART_ERROR errors = uart5Obj.errors;

    uart5Obj.errors = UART_ERROR_NONE;

    /* All errors are cleared, but send the previous error state */
    return errors;
}

bool UART5_AutoBaudQuery( void )
{
    bool autobaudq_check = false;
    if((U5MODE & _U5MODE_ABAUD_MASK) != 0U)
    {
         autobaudq_check = true;
    }
     return autobaudq_check;
}

void UART5_AutoBaudSet( bool enable )
{
    if( enable == true )
    {
        U5MODESET = _U5MODE_ABAUD_MASK;
    }

    /* Turning off ABAUD if it was on can lead to unpredictable behavior, so that
       direction of control is not allowed in this function.                      */
}

static void __attribute__((used)) UART5_FAULT_InterruptHandler (void)
{
    /* Save the error to be reported later */
    uart5Obj.errors = (UART_ERROR)(U5STA & (_U5STA_OERR_MASK | _U5STA_FERR_MASK | _U5STA_PERR_MASK));

    UART5_ErrorClear();

    /* Client must call UARTx_ErrorGet() function to clear the errors */
    if( uart5Obj.rdCallback != NULL )
    {
        uintptr_t rdContext = uart5Obj.rdContext;

        uart5Obj.rdCallback(UART_EVENT_READ_ERROR, rdContext);
    }
}

static void __attribute__((used)) UART5_RX_InterruptHandler (void)
{
    /* Keep reading until there is a character availabe in the RX FIFO */
    while((U5STA & _U5STA_URXDA_MASK) == _U5STA_URXDA_MASK)
    {
        if (UART5_RxPushByte( (uint16_t )(U5RXREG) ) == true)
        {
            UART5_ReadNotificationSend();
        }
        else
        {
            /* UART RX buffer is full */
        }
    }

    /* Clear UART5 RX Interrupt flag */
    IFS2CLR = _IFS2_U5RXIF_MASK;
}

static void __attribute__((used)) UART5_TX_InterruptHandler (void)
{
    uint16_t wrByte;

    /* Check if any data is pending for transmission */
    if (UART5_WritePendingBytesGet() > 0U)
    {
        /* Keep writing to the TX FIFO as long as there is space */
        while((U5STA & _U5STA_UTXBF_MASK) == 0U)
        {
            if (UART5_TxPullByte(&wrByte) == true)
            {
                if (UART5_IS_9BIT_MODE_ENABLED())
                {
                    U5TXREG = wrByte;
                }
                else
                {
                    U5TXREG = (uint8_t)wrByte;
                }

                /* Send notification */
                UART5_WriteNotificationSend();
            }
            else
            {
                /* Nothing to transmit. Disable the data register empty interrupt. */
                UART5_TX_INT_DISABLE();
                break;
            }
        }

        /* Clear UART5TX Interrupt flag */
        IFS2CLR = _IFS2_U5TXIF_MASK;
    }
    else
    {
        /* Nothing to transmit. Disable the data register empty interrupt. */
        UART5_TX_INT_DISABLE();

        /* Clear UART5TX Interrupt flag */
        IFS2CLR = _IFS2_U5TXIF_MASK;
    }
}

void __attribute__((used)) UART_5_InterruptHandler (void)
{
    uint32_t temp = IEC2;
    /* Call Error handler if Error interrupt flag is set */
    if (((IFS2 & _IFS2_U5EIF_MASK) != 0U) && ((temp & _IEC2_U5EIE_MASK) != 0U))
    {
        UART5_FAULT_InterruptHandler();
    }
    temp = IEC2;
    /* Call RX handler if RX interrupt flag is set */
    if (((IFS2 & _IFS2_U5RXIF_MASK) != 0U) && ((temp & _IEC2_U5RXIE_MASK) != 0U))
    {
        UART5_RX_InterruptHandler();
    }
    temp = IEC2;
    /* Call TX handler if TX interrupt flag is set */
    if (((IFS2 & _IFS2_U5TXIF_MASK) != 0U) && ((temp & _IEC2_U5TXIE_MASK) != 0U))
    {
        UART5_TX_InterruptHandler();
    }

}
