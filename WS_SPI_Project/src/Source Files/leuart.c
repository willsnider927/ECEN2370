/**
 * @file leuart.c
 * @author Will Snider
 * @date 7 Sept 2021
 * @brief Contains all the functions of the LEUART peripheral
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************

//** Standard Library includes
#include <string.h>

//** Silicon Labs include files
#include "em_gpio.h"
#include "em_cmu.h"

//** Developer/user include files
#include "leuart.h"
#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// private variables
//***********************************************************************************
uint32_t	rx_done_evt;
uint32_t	tx_done_evt;
static LEUART_STATE_MACHINE leuartSM;
bool		leuart0_tx_busy;

/***************************************************************************//**
 * @brief LEUART driver
 * @details
 *  This module contains all the functions to support the driver's state
 *  machine to transmit a string of data across the LEUART bus.  There are
 *  additional functions to support the Test Driven Development test that
 *  is used to validate the basic set up of the LEUART peripheral.  The
 *  TDD test for this class assumes that the LEUART is connected to the HM-18
 *  BLE module.  These TDD support functions could be used for any TDD test
 *  to validate the correct setup of the LEUART.
 *
 ******************************************************************************/

//***********************************************************************************
// Private functions
//***********************************************************************************
static void leuart_txbl(LEUART_STATE_MACHINE * SM);
static void leuart_txc(LEUART_STATE_MACHINE * SM);


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *  Inits the LEUART peripheral with needed specifications
 * @details
 *  Enables clocking to correct leuart and check proper routing,
 *  sets up all routing and prepares cb's
 * @note
 *  set up for LEUART0
 * @param[in] leuart
 *  Which LEUART to use
 * @param[in] leuart_settings
 *  Settings to be applied to LEUART
 ******************************************************************************/

void leuart_open(LEUART_TypeDef *leuart, LEUART_OPEN_STRUCT *leuart_settings){
  CMU_ClockEnable(cmuClock_LEUART0, true);

  leuart->STARTFRAME = 0x01;
  while(leuart->SYNCBUSY);
  EFM_ASSERT(leuart->STARTFRAME == 0x01);
  leuart->STARTFRAME = 0x00;
  while(leuart->SYNCBUSY);

  LEUART_Init_TypeDef settings;
  settings.baudrate = leuart_settings->baudrate;
  settings.databits = leuart_settings->databits;
  settings.enable = leuartDisable;
  settings.parity = leuart_settings->parity;
  settings.refFreq = leuart_settings->refFreq;
  settings.stopbits = leuart_settings->stopbits;

  rx_done_evt = leuart_settings->rx_done_evt;
  tx_done_evt = leuart_settings->tx_done_evt;

  LEUART_Init(leuart, &settings);

  leuart->ROUTELOC0 |= leuart_settings->rx_loc | leuart_settings->tx_loc;
  leuart->ROUTEPEN |= (LEUART_ROUTEPEN_RXPEN * leuart_settings->rx_pin_en) |
                     (LEUART_ROUTEPEN_TXPEN * leuart_settings->tx_pin_en);

  leuart->CMD = LEUART_CMD_CLEARTX;
  while(leuart->SYNCBUSY);
  leuart->CMD = LEUART_CMD_CLEARRX;
  while(leuart->SYNCBUSY);

  LEUART_Enable(leuart, leuart_settings->enable);
  while(leuart->SYNCBUSY);

  while(!((leuart->STATUS & LEUART_STATUS_RXENS) && (leuart->STATUS & LEUART_STATUS_TXENS)));
  EFM_ASSERT((leuart->STATUS & LEUART_STATUS_RXENS) && (leuart->STATUS & LEUART_STATUS_TXENS));

  NVIC_EnableIRQ(LEUART0_IRQn);
}

/***************************************************************************//**
 * @brief
 * txbl interrupts helper
 * @details
 * sends the next byte of data if that is required or enables txc interrupt to
 * allow for release of SM
 * @note
 *  none
 *
 ******************************************************************************/
void leuart_txbl(LEUART_STATE_MACHINE * SM)
{
  switch (SM->state)
  {
    case WRITE_LEUART:
      if (SM->bytesRequested != SM->bytesDone)
      {
          SM->leuart->TXDATA = SM->data[SM->bytesDone++];
      }
      else
      {
          SM->leuart->IEN |= LEUART_IEN_TXC;
      }
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}
/***************************************************************************//**
 * @brief
 * txc interrupts helper
 * @details
 *  should only be called at end of transmission, ends stateMachine and unchecks busy
 *  for next use
 * @note
 *  none
 *
 ******************************************************************************/
void leuart_txc(LEUART_STATE_MACHINE * SM)
{
  switch (SM->state)
  {
    case WRITE_LEUART:
      SM->state = DONE_LEUART;
      SM->busy = false;
      SM->leuart->IEN &= ~LEUART_IEN_TXC;
      sleep_unblock_mode(LEUART_TX_EM);
      add_scheduled_event(tx_done_evt);
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}
/***************************************************************************//**
 * @brief
 * LEUART interrupts handler
 * @details
 *  calls the correct helper function to maintain statemachine operation
 * @note
 *  none
 *
 ******************************************************************************/

void LEUART0_IRQHandler(void){
  uint32_t intFlags = LEUART0->IF & LEUART0->IEN;
  uint32_t savedInterrupts = LEUART0->IEN;
  LEUART0->IEN &= ~LEUART0->IEN; // Disable all interrupts
  LEUART0->IFC = intFlags;

  if(intFlags & LEUART_IF_TXBL)
  {
      leuart_txbl(&leuartSM);
  }
  if(intFlags & LEUART_IF_TXC)
  {
      leuart_txc(&leuartSM);
      savedInterrupts &= ~(LEUART_IEN_TXC | LEUART_IEN_TXBL);
  }
  LEUART0->IEN |= savedInterrupts;
}

/***************************************************************************//**
 * @brief
 *  Starts leuart communication
 * @details
 *  Starts leuart communication by enabling txbl interrupts and setting up SM struct
 * @note
 *  none
 * @param[in] leuart
 *  leuart to be used
 * @param[in] string
 *  string to be transmitted
 * @param[in]
 *  len of string to be sent
 ******************************************************************************/

void leuart_start(LEUART_TypeDef *leuart, char *string, uint32_t string_len){
  while(leuartSM.busy);

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  //char dataLocal[string_len];
  sleep_block_mode(LEUART_TX_EM);
  //leuartSM.data = dataLocal;
  leuartSM.busy = true;
  leuartSM.bytesDone = 0;
  leuartSM.bytesRequested = string_len;
  strcpy(leuartSM.data,string);
  leuartSM.leuart = leuart;
  leuartSM.state = WRITE_LEUART;

  leuart->IEN |= LEUART_IEN_TXBL;

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   LEUART STATUS function returns the STATUS of the peripheral for the
 *   TDD test
 *
 * @details
 * 	 This function enables the LEUART STATUS register to be provided to
 * 	 a function outside this .c module.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the STATUS register value as an uint32_t value
 *
 ******************************************************************************/

uint32_t leuart_status(LEUART_TypeDef *leuart){
	uint32_t	status_reg;
	status_reg = leuart->STATUS;
	return status_reg;
}

/***************************************************************************//**
 * @brief
 *   LEUART CMD Write sends a command to the CMD register
 *
 * @details
 * 	 This function is used by the TDD test function to program the LEUART
 * 	 for the TDD tests.
 *
 * @note
 *   Before exiting this function to update  the CMD register, it must
 *   perform a SYNCBUSY while loop to ensure that the CMD has by synchronized
 *   to the lower frequency LEUART domain.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] cmd_update
 * 	 The value to write into the CMD register
 *
 ******************************************************************************/

void leuart_cmd_write(LEUART_TypeDef *leuart, uint32_t cmd_update){

	leuart->CMD = cmd_update;
	while(leuart->SYNCBUSY);
}

/***************************************************************************//**
 * @brief
 *   LEUART IF Reset resets all interrupt flag bits that can be cleared
 *   through the Interrupt Flag Clear register
 *
 * @details
 * 	 This function is used by the TDD test program to clear interrupts before
 * 	 the TDD tests and to reset the LEUART interrupts before the TDD
 * 	 exits
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 ******************************************************************************/

void leuart_if_reset(LEUART_TypeDef *leuart){
	leuart->IFC = 0xffffffff;
}

/***************************************************************************//**
 * @brief
 *   LEUART App Transmit Byte transmits a byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a transmit byte, a while statement checking for the TXBL
 *   bit in the Interrupt Flag register is required before writing the
 *   TXDATA register.
 *
 * @param[in] *leuart
 *   Defines the LEUART peripheral to access.
 *
 * @param[in] data_out
 *   Byte to be transmitted by the LEUART peripheral
 *
 ******************************************************************************/

void leuart_app_transmit_byte(LEUART_TypeDef *leuart, uint8_t data_out){
	while (!(leuart->IF & LEUART_IF_TXBL));
	leuart->TXDATA = data_out;
}


/***************************************************************************//**
 * @brief
 *   LEUART App Receive Byte polls a receive byte for the LEUART TDD test
 *
 * @details
 * 	 The BLE module will respond to AT commands if the BLE module is not
 * 	 connected to the phone app.  To validate the minimal functionality
 * 	 of the LEUART peripheral, write and reads to the LEUART will be
 * 	 performed by polling and not interrupts.
 *
 * @note
 *   In polling a receive byte, a while statement checking for the RXDATAV
 *   bit in the Interrupt Flag register is required before reading the
 *   RXDATA register.
 *
 * @param[in] leuart
 *   Defines the LEUART peripheral to access.
 *
 * @return
 * 	 Returns the byte read from the LEUART peripheral
 *
 ******************************************************************************/

uint8_t leuart_app_receive_byte(LEUART_TypeDef *leuart){
	uint8_t leuart_data;
	while (!(leuart->IF & LEUART_IF_RXDATAV));
	leuart_data = leuart->RXDATA;
	return leuart_data;
}
