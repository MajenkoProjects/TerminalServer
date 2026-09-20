/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

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

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for FRES pin ***/
#define FRES_Set()               (LATGSET = (1<<15))
#define FRES_Clear()             (LATGCLR = (1<<15))
#define FRES_Toggle()            (LATGINV= (1<<15))
#define FRES_OutputEnable()      (TRISGCLR = (1<<15))
#define FRES_InputEnable()       (TRISGSET = (1<<15))
#define FRES_Get()               ((PORTG >> 15) & 0x1)
#define FRES_GetLatch()          ((LATG >> 15) & 0x1)
#define FRES_PIN                  GPIO_PIN_RG15

/*** Macros for U6DTR pin ***/
#define U6DTR_Set()               (LATESET = (1<<5))
#define U6DTR_Clear()             (LATECLR = (1<<5))
#define U6DTR_Toggle()            (LATEINV= (1<<5))
#define U6DTR_OutputEnable()      (TRISECLR = (1<<5))
#define U6DTR_InputEnable()       (TRISESET = (1<<5))
#define U6DTR_Get()               ((PORTE >> 5) & 0x1)
#define U6DTR_GetLatch()          ((LATE >> 5) & 0x1)
#define U6DTR_PIN                  GPIO_PIN_RE5

/*** Macros for U6RTS pin ***/
#define U6RTS_Set()               (LATESET = (1<<6))
#define U6RTS_Clear()             (LATECLR = (1<<6))
#define U6RTS_Toggle()            (LATEINV= (1<<6))
#define U6RTS_OutputEnable()      (TRISECLR = (1<<6))
#define U6RTS_InputEnable()       (TRISESET = (1<<6))
#define U6RTS_Get()               ((PORTE >> 6) & 0x1)
#define U6RTS_GetLatch()          ((LATE >> 6) & 0x1)
#define U6RTS_PIN                  GPIO_PIN_RE6

/*** Macros for U3SHTDN pin ***/
#define U3SHTDN_Set()               (LATESET = (1<<7))
#define U3SHTDN_Clear()             (LATECLR = (1<<7))
#define U3SHTDN_Toggle()            (LATEINV= (1<<7))
#define U3SHTDN_OutputEnable()      (TRISECLR = (1<<7))
#define U3SHTDN_InputEnable()       (TRISESET = (1<<7))
#define U3SHTDN_Get()               ((PORTE >> 7) & 0x1)
#define U3SHTDN_GetLatch()          ((LATE >> 7) & 0x1)
#define U3SHTDN_PIN                  GPIO_PIN_RE7

/*** Macros for U3STATUS pin ***/
#define U3STATUS_Set()               (LATCSET = (1<<1))
#define U3STATUS_Clear()             (LATCCLR = (1<<1))
#define U3STATUS_Toggle()            (LATCINV= (1<<1))
#define U3STATUS_OutputEnable()      (TRISCCLR = (1<<1))
#define U3STATUS_InputEnable()       (TRISCSET = (1<<1))
#define U3STATUS_Get()               ((PORTC >> 1) & 0x1)
#define U3STATUS_GetLatch()          ((LATC >> 1) & 0x1)
#define U3STATUS_PIN                  GPIO_PIN_RC1

/*** Macros for U3CTS pin ***/
#define U3CTS_Set()               (LATCSET = (1<<2))
#define U3CTS_Clear()             (LATCCLR = (1<<2))
#define U3CTS_Toggle()            (LATCINV= (1<<2))
#define U3CTS_OutputEnable()      (TRISCCLR = (1<<2))
#define U3CTS_InputEnable()       (TRISCSET = (1<<2))
#define U3CTS_Get()               ((PORTC >> 2) & 0x1)
#define U3CTS_GetLatch()          ((LATC >> 2) & 0x1)
#define U3CTS_PIN                  GPIO_PIN_RC2

/*** Macros for U3DSR pin ***/
#define U3DSR_Set()               (LATCSET = (1<<3))
#define U3DSR_Clear()             (LATCCLR = (1<<3))
#define U3DSR_Toggle()            (LATCINV= (1<<3))
#define U3DSR_OutputEnable()      (TRISCCLR = (1<<3))
#define U3DSR_InputEnable()       (TRISCSET = (1<<3))
#define U3DSR_Get()               ((PORTC >> 3) & 0x1)
#define U3DSR_GetLatch()          ((LATC >> 3) & 0x1)
#define U3DSR_PIN                  GPIO_PIN_RC3

/*** Macros for SDI_MISO pin ***/
#define SDI_MISO_Get()               ((PORTC >> 4) & 0x1)
#define SDI_MISO_GetLatch()          ((LATC >> 4) & 0x1)
#define SDI_MISO_PIN                  GPIO_PIN_RC4

/*** Macros for U6TX pin ***/
#define U6TX_Get()               ((PORTG >> 6) & 0x1)
#define U6TX_GetLatch()          ((LATG >> 6) & 0x1)
#define U6TX_PIN                  GPIO_PIN_RG6

/*** Macros for U3RX pin ***/
#define U3RX_Get()               ((PORTG >> 7) & 0x1)
#define U3RX_GetLatch()          ((LATG >> 7) & 0x1)
#define U3RX_PIN                  GPIO_PIN_RG7

/*** Macros for U3TX pin ***/
#define U3TX_Get()               ((PORTG >> 8) & 0x1)
#define U3TX_GetLatch()          ((LATG >> 8) & 0x1)
#define U3TX_PIN                  GPIO_PIN_RG8

/*** Macros for U6RX pin ***/
#define U6RX_Get()               ((PORTG >> 9) & 0x1)
#define U6RX_GetLatch()          ((LATG >> 9) & 0x1)
#define U6RX_PIN                  GPIO_PIN_RG9

/*** Macros for ESP_EN pin ***/
#define ESP_EN_Set()               (LATASET = (1<<0))
#define ESP_EN_Clear()             (LATACLR = (1<<0))
#define ESP_EN_Toggle()            (LATAINV= (1<<0))
#define ESP_EN_OutputEnable()      (TRISACLR = (1<<0))
#define ESP_EN_InputEnable()       (TRISASET = (1<<0))
#define ESP_EN_Get()               ((PORTA >> 0) & 0x1)
#define ESP_EN_GetLatch()          ((LATA >> 0) & 0x1)
#define ESP_EN_PIN                  GPIO_PIN_RA0

/*** Macros for U3DTR pin ***/
#define U3DTR_Set()               (LATESET = (1<<8))
#define U3DTR_Clear()             (LATECLR = (1<<8))
#define U3DTR_Toggle()            (LATEINV= (1<<8))
#define U3DTR_OutputEnable()      (TRISECLR = (1<<8))
#define U3DTR_InputEnable()       (TRISESET = (1<<8))
#define U3DTR_Get()               ((PORTE >> 8) & 0x1)
#define U3DTR_GetLatch()          ((LATE >> 8) & 0x1)
#define U3DTR_PIN                  GPIO_PIN_RE8

/*** Macros for U3RTS pin ***/
#define U3RTS_Set()               (LATESET = (1<<9))
#define U3RTS_Clear()             (LATECLR = (1<<9))
#define U3RTS_Toggle()            (LATEINV= (1<<9))
#define U3RTS_OutputEnable()      (TRISECLR = (1<<9))
#define U3RTS_InputEnable()       (TRISESET = (1<<9))
#define U3RTS_Get()               ((PORTE >> 9) & 0x1)
#define U3RTS_GetLatch()          ((LATE >> 9) & 0x1)
#define U3RTS_PIN                  GPIO_PIN_RE9

/*** Macros for U5SHTDN pin ***/
#define U5SHTDN_Set()               (LATBSET = (1<<5))
#define U5SHTDN_Clear()             (LATBCLR = (1<<5))
#define U5SHTDN_Toggle()            (LATBINV= (1<<5))
#define U5SHTDN_OutputEnable()      (TRISBCLR = (1<<5))
#define U5SHTDN_InputEnable()       (TRISBSET = (1<<5))
#define U5SHTDN_Get()               ((PORTB >> 5) & 0x1)
#define U5SHTDN_GetLatch()          ((LATB >> 5) & 0x1)
#define U5SHTDN_PIN                  GPIO_PIN_RB5

/*** Macros for U5STATUS pin ***/
#define U5STATUS_Set()               (LATBSET = (1<<4))
#define U5STATUS_Clear()             (LATBCLR = (1<<4))
#define U5STATUS_Toggle()            (LATBINV= (1<<4))
#define U5STATUS_OutputEnable()      (TRISBCLR = (1<<4))
#define U5STATUS_InputEnable()       (TRISBSET = (1<<4))
#define U5STATUS_Get()               ((PORTB >> 4) & 0x1)
#define U5STATUS_GetLatch()          ((LATB >> 4) & 0x1)
#define U5STATUS_PIN                  GPIO_PIN_RB4

/*** Macros for U5CTS pin ***/
#define U5CTS_Set()               (LATBSET = (1<<3))
#define U5CTS_Clear()             (LATBCLR = (1<<3))
#define U5CTS_Toggle()            (LATBINV= (1<<3))
#define U5CTS_OutputEnable()      (TRISBCLR = (1<<3))
#define U5CTS_InputEnable()       (TRISBSET = (1<<3))
#define U5CTS_Get()               ((PORTB >> 3) & 0x1)
#define U5CTS_GetLatch()          ((LATB >> 3) & 0x1)
#define U5CTS_PIN                  GPIO_PIN_RB3

/*** Macros for U5DSR pin ***/
#define U5DSR_Set()               (LATBSET = (1<<2))
#define U5DSR_Clear()             (LATBCLR = (1<<2))
#define U5DSR_Toggle()            (LATBINV= (1<<2))
#define U5DSR_OutputEnable()      (TRISBCLR = (1<<2))
#define U5DSR_InputEnable()       (TRISBSET = (1<<2))
#define U5DSR_Get()               ((PORTB >> 2) & 0x1)
#define U5DSR_GetLatch()          ((LATB >> 2) & 0x1)
#define U5DSR_PIN                  GPIO_PIN_RB2

/*** Macros for PGM_CLOCK pin ***/
#define PGM_CLOCK_Set()               (LATBSET = (1<<1))
#define PGM_CLOCK_Clear()             (LATBCLR = (1<<1))
#define PGM_CLOCK_Toggle()            (LATBINV= (1<<1))
#define PGM_CLOCK_OutputEnable()      (TRISBCLR = (1<<1))
#define PGM_CLOCK_InputEnable()       (TRISBSET = (1<<1))
#define PGM_CLOCK_Get()               ((PORTB >> 1) & 0x1)
#define PGM_CLOCK_GetLatch()          ((LATB >> 1) & 0x1)
#define PGM_CLOCK_PIN                  GPIO_PIN_RB1

/*** Macros for PGM_DATA pin ***/
#define PGM_DATA_Set()               (LATBSET = (1<<0))
#define PGM_DATA_Clear()             (LATBCLR = (1<<0))
#define PGM_DATA_Toggle()            (LATBINV= (1<<0))
#define PGM_DATA_OutputEnable()      (TRISBCLR = (1<<0))
#define PGM_DATA_InputEnable()       (TRISBSET = (1<<0))
#define PGM_DATA_Get()               ((PORTB >> 0) & 0x1)
#define PGM_DATA_GetLatch()          ((LATB >> 0) & 0x1)
#define PGM_DATA_PIN                  GPIO_PIN_RB0

/*** Macros for U5DTR pin ***/
#define U5DTR_Set()               (LATBSET = (1<<6))
#define U5DTR_Clear()             (LATBCLR = (1<<6))
#define U5DTR_Toggle()            (LATBINV= (1<<6))
#define U5DTR_OutputEnable()      (TRISBCLR = (1<<6))
#define U5DTR_InputEnable()       (TRISBSET = (1<<6))
#define U5DTR_Get()               ((PORTB >> 6) & 0x1)
#define U5DTR_GetLatch()          ((LATB >> 6) & 0x1)
#define U5DTR_PIN                  GPIO_PIN_RB6

/*** Macros for U5RTS pin ***/
#define U5RTS_Set()               (LATBSET = (1<<7))
#define U5RTS_Clear()             (LATBCLR = (1<<7))
#define U5RTS_Toggle()            (LATBINV= (1<<7))
#define U5RTS_OutputEnable()      (TRISBCLR = (1<<7))
#define U5RTS_InputEnable()       (TRISBSET = (1<<7))
#define U5RTS_Get()               ((PORTB >> 7) & 0x1)
#define U5RTS_GetLatch()          ((LATB >> 7) & 0x1)
#define U5RTS_PIN                  GPIO_PIN_RB7

/*** Macros for U4SHTDN pin ***/
#define U4SHTDN_Set()               (LATASET = (1<<9))
#define U4SHTDN_Clear()             (LATACLR = (1<<9))
#define U4SHTDN_Toggle()            (LATAINV= (1<<9))
#define U4SHTDN_OutputEnable()      (TRISACLR = (1<<9))
#define U4SHTDN_InputEnable()       (TRISASET = (1<<9))
#define U4SHTDN_Get()               ((PORTA >> 9) & 0x1)
#define U4SHTDN_GetLatch()          ((LATA >> 9) & 0x1)
#define U4SHTDN_PIN                  GPIO_PIN_RA9

/*** Macros for U4STATUS pin ***/
#define U4STATUS_Set()               (LATASET = (1<<10))
#define U4STATUS_Clear()             (LATACLR = (1<<10))
#define U4STATUS_Toggle()            (LATAINV= (1<<10))
#define U4STATUS_OutputEnable()      (TRISACLR = (1<<10))
#define U4STATUS_InputEnable()       (TRISASET = (1<<10))
#define U4STATUS_Get()               ((PORTA >> 10) & 0x1)
#define U4STATUS_GetLatch()          ((LATA >> 10) & 0x1)
#define U4STATUS_PIN                  GPIO_PIN_RA10

/*** Macros for U4CTS pin ***/
#define U4CTS_Set()               (LATBSET = (1<<8))
#define U4CTS_Clear()             (LATBCLR = (1<<8))
#define U4CTS_Toggle()            (LATBINV= (1<<8))
#define U4CTS_OutputEnable()      (TRISBCLR = (1<<8))
#define U4CTS_InputEnable()       (TRISBSET = (1<<8))
#define U4CTS_Get()               ((PORTB >> 8) & 0x1)
#define U4CTS_GetLatch()          ((LATB >> 8) & 0x1)
#define U4CTS_PIN                  GPIO_PIN_RB8

/*** Macros for U4DSR pin ***/
#define U4DSR_Set()               (LATBSET = (1<<9))
#define U4DSR_Clear()             (LATBCLR = (1<<9))
#define U4DSR_Toggle()            (LATBINV= (1<<9))
#define U4DSR_OutputEnable()      (TRISBCLR = (1<<9))
#define U4DSR_InputEnable()       (TRISBSET = (1<<9))
#define U4DSR_Get()               ((PORTB >> 9) & 0x1)
#define U4DSR_GetLatch()          ((LATB >> 9) & 0x1)
#define U4DSR_PIN                  GPIO_PIN_RB9

/*** Macros for U4DTR pin ***/
#define U4DTR_Set()               (LATBSET = (1<<10))
#define U4DTR_Clear()             (LATBCLR = (1<<10))
#define U4DTR_Toggle()            (LATBINV= (1<<10))
#define U4DTR_OutputEnable()      (TRISBCLR = (1<<10))
#define U4DTR_InputEnable()       (TRISBSET = (1<<10))
#define U4DTR_Get()               ((PORTB >> 10) & 0x1)
#define U4DTR_GetLatch()          ((LATB >> 10) & 0x1)
#define U4DTR_PIN                  GPIO_PIN_RB10

/*** Macros for U4RTS pin ***/
#define U4RTS_Set()               (LATBSET = (1<<11))
#define U4RTS_Clear()             (LATBCLR = (1<<11))
#define U4RTS_Toggle()            (LATBINV= (1<<11))
#define U4RTS_OutputEnable()      (TRISBCLR = (1<<11))
#define U4RTS_InputEnable()       (TRISBSET = (1<<11))
#define U4RTS_Get()               ((PORTB >> 11) & 0x1)
#define U4RTS_GetLatch()          ((LATB >> 11) & 0x1)
#define U4RTS_PIN                  GPIO_PIN_RB11

/*** Macros for GPIO3 pin ***/
#define GPIO3_Set()               (LATASET = (1<<1))
#define GPIO3_Clear()             (LATACLR = (1<<1))
#define GPIO3_Toggle()            (LATAINV= (1<<1))
#define GPIO3_OutputEnable()      (TRISACLR = (1<<1))
#define GPIO3_InputEnable()       (TRISASET = (1<<1))
#define GPIO3_Get()               ((PORTA >> 1) & 0x1)
#define GPIO3_GetLatch()          ((LATA >> 1) & 0x1)
#define GPIO3_PIN                  GPIO_PIN_RA1

/*** Macros for U5TX pin ***/
#define U5TX_Get()               ((PORTF >> 13) & 0x1)
#define U5TX_GetLatch()          ((LATF >> 13) & 0x1)
#define U5TX_PIN                  GPIO_PIN_RF13

/*** Macros for U5RX pin ***/
#define U5RX_Get()               ((PORTF >> 12) & 0x1)
#define U5RX_GetLatch()          ((LATF >> 12) & 0x1)
#define U5RX_PIN                  GPIO_PIN_RF12

/*** Macros for GPIO0 pin ***/
#define GPIO0_Set()               (LATBSET = (1<<12))
#define GPIO0_Clear()             (LATBCLR = (1<<12))
#define GPIO0_Toggle()            (LATBINV= (1<<12))
#define GPIO0_OutputEnable()      (TRISBCLR = (1<<12))
#define GPIO0_InputEnable()       (TRISBSET = (1<<12))
#define GPIO0_Get()               ((PORTB >> 12) & 0x1)
#define GPIO0_GetLatch()          ((LATB >> 12) & 0x1)
#define GPIO0_PIN                  GPIO_PIN_RB12

/*** Macros for GPIO1 pin ***/
#define GPIO1_Set()               (LATBSET = (1<<13))
#define GPIO1_Clear()             (LATBCLR = (1<<13))
#define GPIO1_Toggle()            (LATBINV= (1<<13))
#define GPIO1_OutputEnable()      (TRISBCLR = (1<<13))
#define GPIO1_InputEnable()       (TRISBSET = (1<<13))
#define GPIO1_Get()               ((PORTB >> 13) & 0x1)
#define GPIO1_GetLatch()          ((LATB >> 13) & 0x1)
#define GPIO1_PIN                  GPIO_PIN_RB13

/*** Macros for GPIO2 pin ***/
#define GPIO2_Set()               (LATBSET = (1<<14))
#define GPIO2_Clear()             (LATBCLR = (1<<14))
#define GPIO2_Toggle()            (LATBINV= (1<<14))
#define GPIO2_OutputEnable()      (TRISBCLR = (1<<14))
#define GPIO2_InputEnable()       (TRISBSET = (1<<14))
#define GPIO2_Get()               ((PORTB >> 14) & 0x1)
#define GPIO2_GetLatch()          ((LATB >> 14) & 0x1)
#define GPIO2_PIN                  GPIO_PIN_RB14

/*** Macros for SD_CS pin ***/
#define SD_CS_Set()               (LATBSET = (1<<15))
#define SD_CS_Clear()             (LATBCLR = (1<<15))
#define SD_CS_Toggle()            (LATBINV= (1<<15))
#define SD_CS_OutputEnable()      (TRISBCLR = (1<<15))
#define SD_CS_InputEnable()       (TRISBSET = (1<<15))
#define SD_CS_Get()               ((PORTB >> 15) & 0x1)
#define SD_CS_GetLatch()          ((LATB >> 15) & 0x1)
#define SD_CS_PIN                  GPIO_PIN_RB15

/*** Macros for U4RX pin ***/
#define U4RX_Get()               ((PORTD >> 14) & 0x1)
#define U4RX_GetLatch()          ((LATD >> 14) & 0x1)
#define U4RX_PIN                  GPIO_PIN_RD14

/*** Macros for U4TX pin ***/
#define U4TX_Get()               ((PORTD >> 15) & 0x1)
#define U4TX_GetLatch()          ((LATD >> 15) & 0x1)
#define U4TX_PIN                  GPIO_PIN_RD15

/*** Macros for U2RX pin ***/
#define U2RX_Get()               ((PORTF >> 4) & 0x1)
#define U2RX_GetLatch()          ((LATF >> 4) & 0x1)
#define U2RX_PIN                  GPIO_PIN_RF4

/*** Macros for U2TX pin ***/
#define U2TX_Get()               ((PORTF >> 5) & 0x1)
#define U2TX_GetLatch()          ((LATF >> 5) & 0x1)
#define U2TX_PIN                  GPIO_PIN_RF5

/*** Macros for U2SHTDN pin ***/
#define U2SHTDN_Set()               (LATFSET = (1<<3))
#define U2SHTDN_Clear()             (LATFCLR = (1<<3))
#define U2SHTDN_Toggle()            (LATFINV= (1<<3))
#define U2SHTDN_OutputEnable()      (TRISFCLR = (1<<3))
#define U2SHTDN_InputEnable()       (TRISFSET = (1<<3))
#define U2SHTDN_Get()               ((PORTF >> 3) & 0x1)
#define U2SHTDN_GetLatch()          ((LATF >> 3) & 0x1)
#define U2SHTDN_PIN                  GPIO_PIN_RF3

/*** Macros for U1RX pin ***/
#define U1RX_Get()               ((PORTF >> 2) & 0x1)
#define U1RX_GetLatch()          ((LATF >> 2) & 0x1)
#define U1RX_PIN                  GPIO_PIN_RF2

/*** Macros for U1TX pin ***/
#define U1TX_Get()               ((PORTF >> 8) & 0x1)
#define U1TX_GetLatch()          ((LATF >> 8) & 0x1)
#define U1TX_PIN                  GPIO_PIN_RF8

/*** Macros for SCL pin ***/
#define SCL_Get()               ((PORTA >> 2) & 0x1)
#define SCL_GetLatch()          ((LATA >> 2) & 0x1)
#define SCL_PIN                  GPIO_PIN_RA2

/*** Macros for SDA pin ***/
#define SDA_Get()               ((PORTA >> 3) & 0x1)
#define SDA_GetLatch()          ((LATA >> 3) & 0x1)
#define SDA_PIN                  GPIO_PIN_RA3

/*** Macros for ENC_SEL pin ***/
#define ENC_SEL_Set()               (LATASET = (1<<4))
#define ENC_SEL_Clear()             (LATACLR = (1<<4))
#define ENC_SEL_Toggle()            (LATAINV= (1<<4))
#define ENC_SEL_OutputEnable()      (TRISACLR = (1<<4))
#define ENC_SEL_InputEnable()       (TRISASET = (1<<4))
#define ENC_SEL_Get()               ((PORTA >> 4) & 0x1)
#define ENC_SEL_GetLatch()          ((LATA >> 4) & 0x1)
#define ENC_SEL_PIN                  GPIO_PIN_RA4

/*** Macros for ENC_RESET pin ***/
#define ENC_RESET_Set()               (LATASET = (1<<5))
#define ENC_RESET_Clear()             (LATACLR = (1<<5))
#define ENC_RESET_Toggle()            (LATAINV= (1<<5))
#define ENC_RESET_OutputEnable()      (TRISACLR = (1<<5))
#define ENC_RESET_InputEnable()       (TRISASET = (1<<5))
#define ENC_RESET_Get()               ((PORTA >> 5) & 0x1)
#define ENC_RESET_GetLatch()          ((LATA >> 5) & 0x1)
#define ENC_RESET_PIN                  GPIO_PIN_RA5

/*** Macros for ESP_INT pin ***/
#define ESP_INT_Get()               ((PORTA >> 14) & 0x1)
#define ESP_INT_GetLatch()          ((LATA >> 14) & 0x1)
#define ESP_INT_PIN                  GPIO_PIN_RA14

/*** Macros for ENC_INT pin ***/
#define ENC_INT_Get()               ((PORTA >> 15) & 0x1)
#define ENC_INT_GetLatch()          ((LATA >> 15) & 0x1)
#define ENC_INT_PIN                  GPIO_PIN_RA15

/*** Macros for U2STATUS pin ***/
#define U2STATUS_Set()               (LATDSET = (1<<8))
#define U2STATUS_Clear()             (LATDCLR = (1<<8))
#define U2STATUS_Toggle()            (LATDINV= (1<<8))
#define U2STATUS_OutputEnable()      (TRISDCLR = (1<<8))
#define U2STATUS_InputEnable()       (TRISDSET = (1<<8))
#define U2STATUS_Get()               ((PORTD >> 8) & 0x1)
#define U2STATUS_GetLatch()          ((LATD >> 8) & 0x1)
#define U2STATUS_PIN                  GPIO_PIN_RD8

/*** Macros for U2CTS pin ***/
#define U2CTS_Set()               (LATDSET = (1<<9))
#define U2CTS_Clear()             (LATDCLR = (1<<9))
#define U2CTS_Toggle()            (LATDINV= (1<<9))
#define U2CTS_OutputEnable()      (TRISDCLR = (1<<9))
#define U2CTS_InputEnable()       (TRISDSET = (1<<9))
#define U2CTS_Get()               ((PORTD >> 9) & 0x1)
#define U2CTS_GetLatch()          ((LATD >> 9) & 0x1)
#define U2CTS_PIN                  GPIO_PIN_RD9

/*** Macros for SPI_SCK pin ***/
#define SPI_SCK_Get()               ((PORTD >> 10) & 0x1)
#define SPI_SCK_GetLatch()          ((LATD >> 10) & 0x1)
#define SPI_SCK_PIN                  GPIO_PIN_RD10

/*** Macros for ESP_SEL pin ***/
#define ESP_SEL_Set()               (LATDSET = (1<<11))
#define ESP_SEL_Clear()             (LATDCLR = (1<<11))
#define ESP_SEL_Toggle()            (LATDINV= (1<<11))
#define ESP_SEL_OutputEnable()      (TRISDCLR = (1<<11))
#define ESP_SEL_InputEnable()       (TRISDSET = (1<<11))
#define ESP_SEL_Get()               ((PORTD >> 11) & 0x1)
#define ESP_SEL_GetLatch()          ((LATD >> 11) & 0x1)
#define ESP_SEL_PIN                  GPIO_PIN_RD11

/*** Macros for SDI_MOSI pin ***/
#define SDI_MOSI_Get()               ((PORTD >> 0) & 0x1)
#define SDI_MOSI_GetLatch()          ((LATD >> 0) & 0x1)
#define SDI_MOSI_PIN                  GPIO_PIN_RD0

/*** Macros for U2DSR pin ***/
#define U2DSR_Set()               (LATCSET = (1<<13))
#define U2DSR_Clear()             (LATCCLR = (1<<13))
#define U2DSR_Toggle()            (LATCINV= (1<<13))
#define U2DSR_OutputEnable()      (TRISCCLR = (1<<13))
#define U2DSR_InputEnable()       (TRISCSET = (1<<13))
#define U2DSR_Get()               ((PORTC >> 13) & 0x1)
#define U2DSR_GetLatch()          ((LATC >> 13) & 0x1)
#define U2DSR_PIN                  GPIO_PIN_RC13

/*** Macros for U2DTR pin ***/
#define U2DTR_Set()               (LATCSET = (1<<14))
#define U2DTR_Clear()             (LATCCLR = (1<<14))
#define U2DTR_Toggle()            (LATCINV= (1<<14))
#define U2DTR_OutputEnable()      (TRISCCLR = (1<<14))
#define U2DTR_InputEnable()       (TRISCSET = (1<<14))
#define U2DTR_Get()               ((PORTC >> 14) & 0x1)
#define U2DTR_GetLatch()          ((LATC >> 14) & 0x1)
#define U2DTR_PIN                  GPIO_PIN_RC14

/*** Macros for U2RTS pin ***/
#define U2RTS_Set()               (LATDSET = (1<<1))
#define U2RTS_Clear()             (LATDCLR = (1<<1))
#define U2RTS_Toggle()            (LATDINV= (1<<1))
#define U2RTS_OutputEnable()      (TRISDCLR = (1<<1))
#define U2RTS_InputEnable()       (TRISDSET = (1<<1))
#define U2RTS_Get()               ((PORTD >> 1) & 0x1)
#define U2RTS_GetLatch()          ((LATD >> 1) & 0x1)
#define U2RTS_PIN                  GPIO_PIN_RD1

/*** Macros for U1SHTDN pin ***/
#define U1SHTDN_Set()               (LATDSET = (1<<2))
#define U1SHTDN_Clear()             (LATDCLR = (1<<2))
#define U1SHTDN_Toggle()            (LATDINV= (1<<2))
#define U1SHTDN_OutputEnable()      (TRISDCLR = (1<<2))
#define U1SHTDN_InputEnable()       (TRISDSET = (1<<2))
#define U1SHTDN_Get()               ((PORTD >> 2) & 0x1)
#define U1SHTDN_GetLatch()          ((LATD >> 2) & 0x1)
#define U1SHTDN_PIN                  GPIO_PIN_RD2

/*** Macros for U1STATUS pin ***/
#define U1STATUS_Set()               (LATDSET = (1<<3))
#define U1STATUS_Clear()             (LATDCLR = (1<<3))
#define U1STATUS_Toggle()            (LATDINV= (1<<3))
#define U1STATUS_OutputEnable()      (TRISDCLR = (1<<3))
#define U1STATUS_InputEnable()       (TRISDSET = (1<<3))
#define U1STATUS_Get()               ((PORTD >> 3) & 0x1)
#define U1STATUS_GetLatch()          ((LATD >> 3) & 0x1)
#define U1STATUS_PIN                  GPIO_PIN_RD3

/*** Macros for U1CTS pin ***/
#define U1CTS_Set()               (LATDSET = (1<<12))
#define U1CTS_Clear()             (LATDCLR = (1<<12))
#define U1CTS_Toggle()            (LATDINV= (1<<12))
#define U1CTS_OutputEnable()      (TRISDCLR = (1<<12))
#define U1CTS_InputEnable()       (TRISDSET = (1<<12))
#define U1CTS_Get()               ((PORTD >> 12) & 0x1)
#define U1CTS_GetLatch()          ((LATD >> 12) & 0x1)
#define U1CTS_PIN                  GPIO_PIN_RD12

/*** Macros for U1DSR pin ***/
#define U1DSR_Set()               (LATDSET = (1<<13))
#define U1DSR_Clear()             (LATDCLR = (1<<13))
#define U1DSR_Toggle()            (LATDINV= (1<<13))
#define U1DSR_OutputEnable()      (TRISDCLR = (1<<13))
#define U1DSR_InputEnable()       (TRISDSET = (1<<13))
#define U1DSR_Get()               ((PORTD >> 13) & 0x1)
#define U1DSR_GetLatch()          ((LATD >> 13) & 0x1)
#define U1DSR_PIN                  GPIO_PIN_RD13

/*** Macros for U1DTR pin ***/
#define U1DTR_Set()               (LATDSET = (1<<4))
#define U1DTR_Clear()             (LATDCLR = (1<<4))
#define U1DTR_Toggle()            (LATDINV= (1<<4))
#define U1DTR_OutputEnable()      (TRISDCLR = (1<<4))
#define U1DTR_InputEnable()       (TRISDSET = (1<<4))
#define U1DTR_Get()               ((PORTD >> 4) & 0x1)
#define U1DTR_GetLatch()          ((LATD >> 4) & 0x1)
#define U1DTR_PIN                  GPIO_PIN_RD4

/*** Macros for U1RTS pin ***/
#define U1RTS_Set()               (LATDSET = (1<<5))
#define U1RTS_Clear()             (LATDCLR = (1<<5))
#define U1RTS_Toggle()            (LATDINV= (1<<5))
#define U1RTS_OutputEnable()      (TRISDCLR = (1<<5))
#define U1RTS_InputEnable()       (TRISDSET = (1<<5))
#define U1RTS_Get()               ((PORTD >> 5) & 0x1)
#define U1RTS_GetLatch()          ((LATD >> 5) & 0x1)
#define U1RTS_PIN                  GPIO_PIN_RD5

/*** Macros for U1TXLED pin ***/
#define U1TXLED_Set()               (LATDSET = (1<<6))
#define U1TXLED_Clear()             (LATDCLR = (1<<6))
#define U1TXLED_Toggle()            (LATDINV= (1<<6))
#define U1TXLED_OutputEnable()      (TRISDCLR = (1<<6))
#define U1TXLED_InputEnable()       (TRISDSET = (1<<6))
#define U1TXLED_Get()               ((PORTD >> 6) & 0x1)
#define U1TXLED_GetLatch()          ((LATD >> 6) & 0x1)
#define U1TXLED_PIN                  GPIO_PIN_RD6

/*** Macros for U1RXLED pin ***/
#define U1RXLED_Set()               (LATDSET = (1<<7))
#define U1RXLED_Clear()             (LATDCLR = (1<<7))
#define U1RXLED_Toggle()            (LATDINV= (1<<7))
#define U1RXLED_OutputEnable()      (TRISDCLR = (1<<7))
#define U1RXLED_InputEnable()       (TRISDSET = (1<<7))
#define U1RXLED_Get()               ((PORTD >> 7) & 0x1)
#define U1RXLED_GetLatch()          ((LATD >> 7) & 0x1)
#define U1RXLED_PIN                  GPIO_PIN_RD7

/*** Macros for U2TXLED pin ***/
#define U2TXLED_Set()               (LATFSET = (1<<0))
#define U2TXLED_Clear()             (LATFCLR = (1<<0))
#define U2TXLED_Toggle()            (LATFINV= (1<<0))
#define U2TXLED_OutputEnable()      (TRISFCLR = (1<<0))
#define U2TXLED_InputEnable()       (TRISFSET = (1<<0))
#define U2TXLED_Get()               ((PORTF >> 0) & 0x1)
#define U2TXLED_GetLatch()          ((LATF >> 0) & 0x1)
#define U2TXLED_PIN                  GPIO_PIN_RF0

/*** Macros for U2RXLED pin ***/
#define U2RXLED_Set()               (LATFSET = (1<<1))
#define U2RXLED_Clear()             (LATFCLR = (1<<1))
#define U2RXLED_Toggle()            (LATFINV= (1<<1))
#define U2RXLED_OutputEnable()      (TRISFCLR = (1<<1))
#define U2RXLED_InputEnable()       (TRISFSET = (1<<1))
#define U2RXLED_Get()               ((PORTF >> 1) & 0x1)
#define U2RXLED_GetLatch()          ((LATF >> 1) & 0x1)
#define U2RXLED_PIN                  GPIO_PIN_RF1

/*** Macros for U4TXLED pin ***/
#define U4TXLED_Set()               (LATGSET = (1<<1))
#define U4TXLED_Clear()             (LATGCLR = (1<<1))
#define U4TXLED_Toggle()            (LATGINV= (1<<1))
#define U4TXLED_OutputEnable()      (TRISGCLR = (1<<1))
#define U4TXLED_InputEnable()       (TRISGSET = (1<<1))
#define U4TXLED_Get()               ((PORTG >> 1) & 0x1)
#define U4TXLED_GetLatch()          ((LATG >> 1) & 0x1)
#define U4TXLED_PIN                  GPIO_PIN_RG1

/*** Macros for U4RXLED pin ***/
#define U4RXLED_Set()               (LATGSET = (1<<0))
#define U4RXLED_Clear()             (LATGCLR = (1<<0))
#define U4RXLED_Toggle()            (LATGINV= (1<<0))
#define U4RXLED_OutputEnable()      (TRISGCLR = (1<<0))
#define U4RXLED_InputEnable()       (TRISGSET = (1<<0))
#define U4RXLED_Get()               ((PORTG >> 0) & 0x1)
#define U4RXLED_GetLatch()          ((LATG >> 0) & 0x1)
#define U4RXLED_PIN                  GPIO_PIN_RG0

/*** Macros for U5TXLED pin ***/
#define U5TXLED_Set()               (LATASET = (1<<6))
#define U5TXLED_Clear()             (LATACLR = (1<<6))
#define U5TXLED_Toggle()            (LATAINV= (1<<6))
#define U5TXLED_OutputEnable()      (TRISACLR = (1<<6))
#define U5TXLED_InputEnable()       (TRISASET = (1<<6))
#define U5TXLED_Get()               ((PORTA >> 6) & 0x1)
#define U5TXLED_GetLatch()          ((LATA >> 6) & 0x1)
#define U5TXLED_PIN                  GPIO_PIN_RA6

/*** Macros for U5RXLED pin ***/
#define U5RXLED_Set()               (LATASET = (1<<7))
#define U5RXLED_Clear()             (LATACLR = (1<<7))
#define U5RXLED_Toggle()            (LATAINV= (1<<7))
#define U5RXLED_OutputEnable()      (TRISACLR = (1<<7))
#define U5RXLED_InputEnable()       (TRISASET = (1<<7))
#define U5RXLED_Get()               ((PORTA >> 7) & 0x1)
#define U5RXLED_GetLatch()          ((LATA >> 7) & 0x1)
#define U5RXLED_PIN                  GPIO_PIN_RA7

/*** Macros for U3TXLED pin ***/
#define U3TXLED_Set()               (LATESET = (1<<0))
#define U3TXLED_Clear()             (LATECLR = (1<<0))
#define U3TXLED_Toggle()            (LATEINV= (1<<0))
#define U3TXLED_OutputEnable()      (TRISECLR = (1<<0))
#define U3TXLED_InputEnable()       (TRISESET = (1<<0))
#define U3TXLED_Get()               ((PORTE >> 0) & 0x1)
#define U3TXLED_GetLatch()          ((LATE >> 0) & 0x1)
#define U3TXLED_PIN                  GPIO_PIN_RE0

/*** Macros for U3RXLED pin ***/
#define U3RXLED_Set()               (LATESET = (1<<1))
#define U3RXLED_Clear()             (LATECLR = (1<<1))
#define U3RXLED_Toggle()            (LATEINV= (1<<1))
#define U3RXLED_OutputEnable()      (TRISECLR = (1<<1))
#define U3RXLED_InputEnable()       (TRISESET = (1<<1))
#define U3RXLED_Get()               ((PORTE >> 1) & 0x1)
#define U3RXLED_GetLatch()          ((LATE >> 1) & 0x1)
#define U3RXLED_PIN                  GPIO_PIN_RE1

/*** Macros for U6TXLED pin ***/
#define U6TXLED_Set()               (LATGSET = (1<<14))
#define U6TXLED_Clear()             (LATGCLR = (1<<14))
#define U6TXLED_Toggle()            (LATGINV= (1<<14))
#define U6TXLED_OutputEnable()      (TRISGCLR = (1<<14))
#define U6TXLED_InputEnable()       (TRISGSET = (1<<14))
#define U6TXLED_Get()               ((PORTG >> 14) & 0x1)
#define U6TXLED_GetLatch()          ((LATG >> 14) & 0x1)
#define U6TXLED_PIN                  GPIO_PIN_RG14

/*** Macros for U6RXLED pin ***/
#define U6RXLED_Set()               (LATGSET = (1<<12))
#define U6RXLED_Clear()             (LATGCLR = (1<<12))
#define U6RXLED_Toggle()            (LATGINV= (1<<12))
#define U6RXLED_OutputEnable()      (TRISGCLR = (1<<12))
#define U6RXLED_InputEnable()       (TRISGSET = (1<<12))
#define U6RXLED_Get()               ((PORTG >> 12) & 0x1)
#define U6RXLED_GetLatch()          ((LATG >> 12) & 0x1)
#define U6RXLED_PIN                  GPIO_PIN_RG12

/*** Macros for U6SHTDN pin ***/
#define U6SHTDN_Set()               (LATGSET = (1<<13))
#define U6SHTDN_Clear()             (LATGCLR = (1<<13))
#define U6SHTDN_Toggle()            (LATGINV= (1<<13))
#define U6SHTDN_OutputEnable()      (TRISGCLR = (1<<13))
#define U6SHTDN_InputEnable()       (TRISGSET = (1<<13))
#define U6SHTDN_Get()               ((PORTG >> 13) & 0x1)
#define U6SHTDN_GetLatch()          ((LATG >> 13) & 0x1)
#define U6SHTDN_PIN                  GPIO_PIN_RG13

/*** Macros for U6STATUS pin ***/
#define U6STATUS_Set()               (LATESET = (1<<2))
#define U6STATUS_Clear()             (LATECLR = (1<<2))
#define U6STATUS_Toggle()            (LATEINV= (1<<2))
#define U6STATUS_OutputEnable()      (TRISECLR = (1<<2))
#define U6STATUS_InputEnable()       (TRISESET = (1<<2))
#define U6STATUS_Get()               ((PORTE >> 2) & 0x1)
#define U6STATUS_GetLatch()          ((LATE >> 2) & 0x1)
#define U6STATUS_PIN                  GPIO_PIN_RE2

/*** Macros for U6CTS pin ***/
#define U6CTS_Set()               (LATESET = (1<<3))
#define U6CTS_Clear()             (LATECLR = (1<<3))
#define U6CTS_Toggle()            (LATEINV= (1<<3))
#define U6CTS_OutputEnable()      (TRISECLR = (1<<3))
#define U6CTS_InputEnable()       (TRISESET = (1<<3))
#define U6CTS_Get()               ((PORTE >> 3) & 0x1)
#define U6CTS_GetLatch()          ((LATE >> 3) & 0x1)
#define U6CTS_PIN                  GPIO_PIN_RE3

/*** Macros for U6DSR pin ***/
#define U6DSR_Set()               (LATESET = (1<<4))
#define U6DSR_Clear()             (LATECLR = (1<<4))
#define U6DSR_Toggle()            (LATEINV= (1<<4))
#define U6DSR_OutputEnable()      (TRISECLR = (1<<4))
#define U6DSR_InputEnable()       (TRISESET = (1<<4))
#define U6DSR_Get()               ((PORTE >> 4) & 0x1)
#define U6DSR_GetLatch()          ((LATE >> 4) & 0x1)
#define U6DSR_PIN                  GPIO_PIN_RE4


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/


#define    GPIO_PORT_A   (0)
#define    GPIO_PORT_B   (1)
#define    GPIO_PORT_C   (2)
#define    GPIO_PORT_D   (3)
#define    GPIO_PORT_E   (4)
#define    GPIO_PORT_F   (5)
#define    GPIO_PORT_G   (6)
typedef uint32_t GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/
#define    GPIO_PIN_RA0   (0)
#define    GPIO_PIN_RA1   (1)
#define    GPIO_PIN_RA2   (2)
#define    GPIO_PIN_RA3   (3)
#define    GPIO_PIN_RA4   (4)
#define    GPIO_PIN_RA5   (5)
#define    GPIO_PIN_RA6   (6)
#define    GPIO_PIN_RA7   (7)
#define    GPIO_PIN_RA9   (9)
#define    GPIO_PIN_RA10   (10)
#define    GPIO_PIN_RA14   (14)
#define    GPIO_PIN_RA15   (15)
#define    GPIO_PIN_RB0   (16)
#define    GPIO_PIN_RB1   (17)
#define    GPIO_PIN_RB2   (18)
#define    GPIO_PIN_RB3   (19)
#define    GPIO_PIN_RB4   (20)
#define    GPIO_PIN_RB5   (21)
#define    GPIO_PIN_RB6   (22)
#define    GPIO_PIN_RB7   (23)
#define    GPIO_PIN_RB8   (24)
#define    GPIO_PIN_RB9   (25)
#define    GPIO_PIN_RB10   (26)
#define    GPIO_PIN_RB11   (27)
#define    GPIO_PIN_RB12   (28)
#define    GPIO_PIN_RB13   (29)
#define    GPIO_PIN_RB14   (30)
#define    GPIO_PIN_RB15   (31)
#define    GPIO_PIN_RC1   (33)
#define    GPIO_PIN_RC2   (34)
#define    GPIO_PIN_RC3   (35)
#define    GPIO_PIN_RC4   (36)
#define    GPIO_PIN_RC12   (44)
#define    GPIO_PIN_RC13   (45)
#define    GPIO_PIN_RC14   (46)
#define    GPIO_PIN_RC15   (47)
#define    GPIO_PIN_RD0   (48)
#define    GPIO_PIN_RD1   (49)
#define    GPIO_PIN_RD2   (50)
#define    GPIO_PIN_RD3   (51)
#define    GPIO_PIN_RD4   (52)
#define    GPIO_PIN_RD5   (53)
#define    GPIO_PIN_RD6   (54)
#define    GPIO_PIN_RD7   (55)
#define    GPIO_PIN_RD8   (56)
#define    GPIO_PIN_RD9   (57)
#define    GPIO_PIN_RD10   (58)
#define    GPIO_PIN_RD11   (59)
#define    GPIO_PIN_RD12   (60)
#define    GPIO_PIN_RD13   (61)
#define    GPIO_PIN_RD14   (62)
#define    GPIO_PIN_RD15   (63)
#define    GPIO_PIN_RE0   (64)
#define    GPIO_PIN_RE1   (65)
#define    GPIO_PIN_RE2   (66)
#define    GPIO_PIN_RE3   (67)
#define    GPIO_PIN_RE4   (68)
#define    GPIO_PIN_RE5   (69)
#define    GPIO_PIN_RE6   (70)
#define    GPIO_PIN_RE7   (71)
#define    GPIO_PIN_RE8   (72)
#define    GPIO_PIN_RE9   (73)
#define    GPIO_PIN_RF0   (80)
#define    GPIO_PIN_RF1   (81)
#define    GPIO_PIN_RF2   (82)
#define    GPIO_PIN_RF3   (83)
#define    GPIO_PIN_RF4   (84)
#define    GPIO_PIN_RF5   (85)
#define    GPIO_PIN_RF8   (88)
#define    GPIO_PIN_RF12   (92)
#define    GPIO_PIN_RF13   (93)
#define    GPIO_PIN_RG0   (96)
#define    GPIO_PIN_RG1   (97)
#define    GPIO_PIN_RG2   (98)
#define    GPIO_PIN_RG3   (99)
#define    GPIO_PIN_RG6   (102)
#define    GPIO_PIN_RG7   (103)
#define    GPIO_PIN_RG8   (104)
#define    GPIO_PIN_RG9   (105)
#define    GPIO_PIN_RG12   (108)
#define    GPIO_PIN_RG13   (109)
#define    GPIO_PIN_RG14   (110)
#define    GPIO_PIN_RG15   (111)

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
#define    GPIO_PIN_NONE     (-1)

typedef uint32_t GPIO_PIN;

typedef enum
{
  CN0_PIN = 1 << 0,
  CN1_PIN = 1 << 1,
  CN2_PIN = 1 << 2,
  CN3_PIN = 1 << 3,
  CN4_PIN = 1 << 4,
  CN5_PIN = 1 << 5,
  CN6_PIN = 1 << 6,
  CN7_PIN = 1 << 7,
  CN8_PIN = 1 << 8,
  CN9_PIN = 1 << 9,
  CN10_PIN = 1 << 10,
  CN11_PIN = 1 << 11,
  CN12_PIN = 1 << 12,
  CN13_PIN = 1 << 13,
  CN14_PIN = 1 << 14,
  CN15_PIN = 1 << 15,
  CN16_PIN = 1 << 16,
  CN17_PIN = 1 << 17,
  CN18_PIN = 1 << 18,
  CN19_PIN = 1 << 19,
  CN20_PIN = 1 << 20,
  CN21_PIN = 1 << 21,
}CN_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite((GPIO_PORT)(pin>>4), (uint32_t)(0x1UL) << (pin & 0xFU), (uint32_t)(value) << (pin & 0xFU));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead((GPIO_PORT)(pin>>4U))) >> (pin & 0xFU)) & 0x1U);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead((GPIO_PORT)(pin>>4U)) >> (pin & 0xFU)) & 0x1U);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((GPIO_PORT)(pin>>4U), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((GPIO_PORT)(pin>>4U), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((GPIO_PORT)(pin>>4U), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((GPIO_PORT)(pin>>4U), 0x1UL << (pin & 0xFU));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((GPIO_PORT)(pin>>4U), 0x1UL << (pin & 0xFU));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
