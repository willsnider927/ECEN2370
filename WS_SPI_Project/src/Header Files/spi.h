/*
 * spi.h
 *
 *  Created on: Nov 9, 2021
 *      Author: will
 */

#ifndef SPI_H_
#define SPI_H_

//***********************************************************************************
// Include files
//***********************************************************************************
/* System include statements */
#include "em_usart.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_assert.h"
/* Silicon Labs include statements */


/* The developer's include statements */
#include "sleep_routines.h"
#include "brd_config.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define DUMMY_DATA     0x00
#define USART_EM_BLOCK EM2

//***********************************************************************************
// global variables
//***********************************************************************************
typedef enum
{
  SEND_REG_SPI,
  WRITE_DATA_SPI,
  READ_DATA_SPI,
  DONE_STATE_SPI,
} USART_DEFINED_STATES;

typedef struct {
  USART_Enable_TypeDef    enable;
  bool                    master;
  USART_ClockMode_TypeDef clockmode;
  uint32_t                refFreq;
  uint32_t                baudRate;
  USART_Databits_TypeDef  dataBits;
  bool                    msbf;
  bool                    prsRxEnable;
  USART_PRS_Channel_t     prsRxCh;
  bool                    autoTx;
  bool                    autoCsEnable;
  uint8_t                 autoCsHold;
  uint8_t                 autoCsSetup;
  USART_Stopbits_TypeDef  stopBits;
  uint32_t                miso_pin_route;
  uint32_t                mosi_pin_route;
  uint32_t                clk_pin_route;
  uint32_t                cs_pin_route;
  bool                    miso_pin_en;
  bool                    mosi_pin_en;
  bool                    clk_pin_en;
  bool                    cs_pin_en;
  GPIO_Port_TypeDef       csPort;
  uint32_t                csPin;
} USART_OPEN_STRUCT;

typedef struct {
  USART_TypeDef         *usart;
  USART_DEFINED_STATES  state;
  int                   bytesRequested;
  int                   bytesSent;
  int                   bytesReceived;
  uint32_t              regAddress;
  uint32_t              *rdata;
  uint32_t              *tdata;
  int                   RW;
  volatile bool         busy;
  uint32_t              cb;
} USART_STATE_MACHINE;

//***********************************************************************************
// function prototypes
//***********************************************************************************

void usart_open(USART_TypeDef *usart, USART_OPEN_STRUCT *usart_setup);
void usart_start(USART_TypeDef *usart, uint32_t RW, uint32_t regAddress, uint32_t *data, int cb, int bytesRequested);
bool isSm3Busy();
uint32_t *ret_sm3_read();

void USART3_RX_IRQHandler(void);
void USART3_TX_IRQHandler(void);

#endif /* HEADER_FILES_SPI_H_ */
