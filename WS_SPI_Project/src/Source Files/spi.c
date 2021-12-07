/**
 * @file spi.c
 * @author Will Snider
 * @date 09 Nov 2021
 * @brief SPI Driver
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "spi.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// Private variables
//***********************************************************************************
static USART_STATE_MACHINE sm3;
static uint32_t *sm3_read_res;
static GPIO_Port_TypeDef cs_port;
static uint32_t cs_pin;
//***********************************************************************************
// Private functions
//***********************************************************************************
static void usart_rxdatav(USART_STATE_MACHINE * sm);
static void usart_txc(USART_STATE_MACHINE * sm);
static void usart_txbl(USART_STATE_MACHINE * sm);
//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * initializes an spi bus
 *
 * @details
 * initializes the spi bus, enables routing of spi and clocks;
 * sets specific spi settings needed for the peripheral requesting initialization
 *
 * @param[in] usart
 *    The bus being initialized, USART0-3
 *
 * @param[in] usart_setup
 *    The struct containing all the information needed to initialize the bus
 *
 * @note
 * This function is called by the peripheral, 4 spi peripherals are available.
 * Settings are specific to the peripheral. currently only fully functional for
 * USART3
 ******************************************************************************/
void usart_open(USART_TypeDef *usart, USART_OPEN_STRUCT *usart_setup)
{
  if(usart == USART0) CMU_ClockEnable(cmuClock_USART0, true);
  else if (usart == USART1) CMU_ClockEnable(cmuClock_USART1, true);
  else if (usart == USART2) CMU_ClockEnable(cmuClock_USART2, true);
  else CMU_ClockEnable(cmuClock_USART3, true);

  if ((usart->IF & 0x01) == 0)
  {
      usart->IFS = 0x01;
      EFM_ASSERT(usart->IF & 0x01);
      usart->IFC = 0x01;
  } else
  {
      usart->IFC = 0x01;
      EFM_ASSERT(!(usart->IF & 0x01));
  }

  USART_InitSync_TypeDef usart_values;

  usart_values.autoCsEnable = usart_setup->autoCsEnable;
  usart_values.autoCsHold = usart_setup->autoCsHold;
  usart_values.autoCsSetup = usart_setup->autoCsSetup;
  usart_values.autoTx = usart_setup->autoTx;
  usart_values.baudrate = usart_setup->baudRate;
  usart_values.clockMode = usart_setup->clockmode;
  usart_values.databits = usart_setup->dataBits;
  usart_values.enable = usartDisable;
  usart_values.master = usart_setup->master;
  usart_values.msbf = usart_setup->msbf;
  usart_values.prsRxCh = usart_setup->prsRxCh;
  usart_values.prsRxEnable = usart_setup->prsRxEnable;
  usart_values.refFreq = usart_setup->refFreq;

  USART_InitSync(usart, &usart_values);

  usart->ROUTELOC0 = usart_setup->mosi_pin_route | usart_setup->miso_pin_route
                    |usart_setup->clk_pin_route  | usart_setup->cs_pin_route;
  usart->ROUTEPEN  = (usart_setup->mosi_pin_en * USART_ROUTEPEN_TXPEN) |
                     (usart_setup->miso_pin_en * USART_ROUTEPEN_RXPEN)    |
                     (usart_setup->cs_pin_en   * USART_ROUTEPEN_CSPEN)    |
                     (usart_setup->clk_pin_en  * USART_ROUTEPEN_CLKPEN);

  usart->IFC = USART_IEN_RXDATAV;
  usart->IEN |= USART_IEN_RXDATAV;
  usart->CMD = USART_CMD_CLEARTX;
  usart->CMD = USART_CMD_CLEARRX;
  //usart->FRAME |= usart_setup->stopBits;
  USART_Enable(usart,usart_setup->enable);

  while(!((usart->STATUS & USART_STATUS_RXENS) && (usart->STATUS & USART_STATUS_TXENS)));
  EFM_ASSERT((usart->STATUS & USART_STATUS_RXENS) && (usart->STATUS & USART_STATUS_TXENS));

  if (usart == USART0)
  {
      NVIC_EnableIRQ(USART0_TX_IRQn);
      NVIC_EnableIRQ(USART0_RX_IRQn);
  } else if (usart == USART1)
  {
      NVIC_EnableIRQ(USART1_TX_IRQn);
      NVIC_EnableIRQ(USART1_RX_IRQn);
  } else if (usart == USART2)
  {
      NVIC_EnableIRQ(USART2_TX_IRQn);
      NVIC_EnableIRQ(USART2_RX_IRQn);
  } else
  {
      NVIC_EnableIRQ(USART3_TX_IRQn);
      NVIC_EnableIRQ(USART3_RX_IRQn);
  }
  cs_port = usart_setup->csPort;
  cs_pin = usart_setup->csPin;
  sm3.usart = USART3;
}

/***************************************************************************//**
 * @brief
 * function to return status of spi operation
 *
 * @details
 * returns whether sm3 is busy or ready to be restarted
 *
 * @param[out] busy
 *  whether the statemachine is ready or not
 *
 * @note
 *  only nescesarry for the test function as application can be handled
 *  by schedular
 ******************************************************************************/
bool isSm3Busy()
{
    return sm3.busy;
}

/***************************************************************************//**
 * @brief
 * function to return result of an spi read
 *
 * @details
 * returns the pointer to the array of bytes recieved
 *
 * @param[out] read_result
 * the array of size bytesRequested containing the values read from the read operation
 *
 * @note
 *  statemachine must go not busy once for data to be valid otherwise result of previous
 *  read would be returned
 ******************************************************************************/
uint32_t *ret_sm3_read()
{
  return sm3_read_res;
}

/***************************************************************************//**
 * @brief
 * function to start spi operation
 *
 * @details
 * starts spi operation with the settings passed in, fully
 * interrupt driven
 *
 * @param[in] usart
 *  usart to be used
 *
 * @param[in] RW
 *  Read or Write opeation W = 0, R = 0x80
 *
 * @param[in] regAddress
 *  register to be communicated with on the peripherals end
 *
 * @param[in] data
 *  the data to be written out, does not matter in case of write
 *
 * @param[in] cb
 *  callback to be serviced at end of operation
 *
 * @param[in] bytesRequested
 *  the amount of bytes to be received or sent
 *
 * @note
 *  only functional with USART3
 ******************************************************************************/
void usart_start(USART_TypeDef *usart, uint32_t RW, uint32_t regAddress, uint32_t *data, int cb, int bytesRequested)
{
  USART_STATE_MACHINE *sm = &sm3;
  uint32_t tdata[bytesRequested];
  uint32_t rdata[bytesRequested];
  if (!RW) for (int i = 0; i < bytesRequested;i++) tdata[i] = data[i];
  sm3_read_res = rdata;

  while(sm->busy);
  sm->busy = true;
  sm->cb = cb;
  sm->rdata = sm3_read_res;
  sm->tdata = tdata;
  sm->regAddress = regAddress;
  sm->state = SEND_REG_SPI;
  sm->RW = RW;
  sm->bytesSent = 0;
  sm->bytesReceived = 0;
  sm->bytesRequested = bytesRequested+1;

  sleep_block_mode(USART_EM_BLOCK);
  usart->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  GPIO_PinOutClear(cs_port,cs_pin);
  if (RW) usart->IEN |= USART_IEN_RXDATAV;
  else usart->IEN &= ~USART_IEN_RXDATAV;
  usart->IEN |= USART_IEN_TXBL;
}

/***************************************************************************//**
 * @brief
 * TXBL interrupt helper function
 *
 * @details
 * function to service txbl interrupts and progress the statemachine
 *
 * @param[in] sm
 *  the statemachine to be progressed including the spi line to use
 *
 * @note
 *  none
 ******************************************************************************/
static void usart_txbl(USART_STATE_MACHINE * sm)
{
  switch(sm->state)
  {
    case SEND_REG_SPI:
      if (sm->RW)
      {
          sm->state = READ_DATA_SPI;
      }
      else sm->state = WRITE_DATA_SPI;
      sm->bytesSent++;
      sm->usart->TXDATA = sm->regAddress | sm->RW;
      break;
    case WRITE_DATA_SPI:
      if (sm->bytesSent != sm->bytesRequested)
      {
          sm->usart->TXDATA = *(sm->tdata++);
          sm->bytesSent++;
      }
      break;
    case READ_DATA_SPI:
      if (sm->bytesSent == sm->bytesRequested) sm->usart->IEN &= ~USART_IEN_TXBL;
      else
      {
          sm->usart->TXDATA = DUMMY_DATA;
          sm->bytesSent++;
      }
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * TXC interrupt helper function
 *
 * @details
 * function to service txc interrupts and progress the statemachine
 *
 * @param[in] sm
 *  the statemachine to be progressed including the spi line to use
 *
 * @note
 *  none
 ******************************************************************************/
static void usart_txc(USART_STATE_MACHINE * sm)
{
  switch(sm->state)
  {
    case WRITE_DATA_SPI:
      sm->state = DONE_STATE_SPI;
      sm->usart->CMD = USART_CMD_CLEARRX;
      sleep_unblock_mode(USART_EM_BLOCK);
      add_scheduled_event(sm->cb);
      GPIO_PinOutSet(cs_port,cs_pin);
      sm->busy = false;
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * RXDATAV interrupt helper function
 *
 * @details
 * function to service rxdatav interrupts and progress the statemachine
 *
 * @param[in] sm
 *  the statemachine to be progressed including the spi line to use
 *
 * @note
 *  none
 ******************************************************************************/
static void usart_rxdatav(USART_STATE_MACHINE * sm)
{
 switch(sm->state)
 {
   case READ_DATA_SPI:
     if (sm->bytesReceived == 0)
     {
         *sm->rdata= sm->usart->RXDATA;
         sm->bytesReceived++;
         break;
     }
     *(sm->rdata++) = sm->usart->RXDATA;
     sm->bytesReceived++;
     if (sm->bytesReceived == sm->bytesRequested)
     {
         sm->state = DONE_STATE_SPI;
         add_scheduled_event(sm->cb);
         sleep_unblock_mode(USART_EM_BLOCK);
         EFM_ASSERT(sm->bytesSent == sm->bytesRequested);
         GPIO_PinOutSet(cs_port,cs_pin);
         sm3.busy = false;
     }
     break;
   default:
     EFM_ASSERT(false);
     break;
 }
}

/***************************************************************************//**
 * @brief
 *  USART3 TX interrupt handler
 *
 * @details
 *  interrupt handler for all tx interrupts on USART3
 *  calls needed helper functions and progresses statemachine
 *
 * @note
 *  handles only tx
 ******************************************************************************/
void USART3_TX_IRQHandler(void)
{
  uint32_t intFlags = USART3->IF & USART3->IEN;
  uint32_t savedInterrupts = USART3->IEN;
  USART3->IEN &= ~USART3->IEN; // Disable all interrupts
  USART3->IFC = intFlags;

  if(intFlags & USART_IF_TXBL)
  {
      usart_txbl(&sm3);
      if (sm3.bytesSent == sm3.bytesRequested)
      {
          if (!sm3.RW) savedInterrupts |= USART_IEN_TXC;
          savedInterrupts &= ~USART_IEN_TXBL;
      }
  }
  if(intFlags & USART_IF_TXC)
  {
      usart_txc(&sm3);
      savedInterrupts &= ~USART_IEN_TXC;
  }
  USART3->IEN = savedInterrupts;
}

/***************************************************************************//**
 * @brief
 *  USART3 RX interrupt handler
 *
 * @details
 *  interrupt handler for all rx interrupts on USART3
 *  calls needed helper functions and progresses statemachine
 *
 * @note
 *  handles only rx
 ******************************************************************************/
void USART3_RX_IRQHandler(void)
{
  uint32_t intFlags = USART3->IF & USART3->IEN;
  uint32_t savedInterrupts = USART3->IEN;
  USART3->IEN &= ~USART3->IEN; // Disable all interrupts
  USART3->IFC = intFlags;

  if(intFlags & USART_IF_RXDATAV)
  {
      usart_rxdatav(&sm3);
      if (sm3.bytesRequested == sm3.bytesReceived)
      {
          savedInterrupts &= ~USART_IEN_RXDATAV;
      }
  }
  USART3->IEN = savedInterrupts;
}
