/**
 * @file i2c.c
 * @author Will Snider
 * @date 10/05/2021
 * @brief
 * file containing all i2c driver functions
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "i2c.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static I2C_STATE_MACHINE stateMachine0;
static I2C_STATE_MACHINE stateMachine1;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void i2c_bus_reset(I2C_TypeDef *i2c);

/***************************************************************************//**
 * @brief
 *  resets the i2c bus of the specified input
 *
 * @details
 *  resets the i2c bus of the specified input, I2C0 or I2C1
 *
 * @param[in] i2c
 *  the specific i2c bus to be reset
 *
 * @note
 *  this function has to be called between every i2c transfer
 *
 *
 ******************************************************************************/
static void i2c_bus_reset(I2C_TypeDef *i2c)
{
  uint32_t intEn;

  i2c->CMD = I2C_CMD_ABORT;
  intEn = i2c->IEN;
  i2c->IEN &= ~i2c->IEN;
  i2c->IFC = i2c->IF;
  i2c->CMD = I2C_CMD_CLEARTX;

  i2c->CMD = I2C_CMD_START | I2C_CMD_STOP;
  while(!(i2c->IF & I2C_IF_MSTOP));

  i2c->IFC = i2c->IF;
  i2c->CMD = I2C_CMD_ABORT;
  i2c->IEN = intEn;
}

static void i2c_ack(I2C_STATE_MACHINE * stateMachine);
static void i2c_rxdata(I2C_STATE_MACHINE * stateMachine);
static void i2c_mstop(I2C_STATE_MACHINE * stateMachine);

//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * initializes the the specified i2c bus
 *
 * @details
 * initializes the specified i2c bus, enables routing of i2c and needed clocks,
 * sets specific i2c settings needed for the peripheral requesting initialization
 *
 * @param[in] i2c, i2c_setup
 *  i2c:
 *    The bus being initialized, I2C0 or I2C1
 *  i2c_setup:
 *    The struct containing all the information needed to initialize the bus
 *
 * @note
 * This function is called by the peripheral, only two i2c peripheral are available
 * settings are specific to the peripheral
 *
 *
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup)
{
  if (i2c == I2C0) CMU_ClockEnable(cmuClock_I2C0, true);
  else CMU_ClockEnable(cmuClock_I2C1, true);

  if ((i2c->IF & 0x01)==0)
  {
      i2c->IFS = 0x01;
      EFM_ASSERT(i2c->IF & 0x01);
      i2c->IFC = 0x01;
  }
  else
  {
      i2c->IFC = 0x01;
      EFM_ASSERT(!(i2c->IF & 0x01));
  }

  I2C_Init_TypeDef i2c_init_struct;
  i2c_init_struct.enable = i2c_setup->enable;
  i2c_init_struct.clhr = i2c_setup->clhr;
  i2c_init_struct.freq = i2c_setup->freq;
  i2c_init_struct.master = i2c_setup->master;
  i2c_init_struct.refFreq = i2c_setup->refFreq;

  I2C_Init(i2c, &i2c_init_struct);

  i2c->ROUTELOC0 = (i2c_setup->scl_pin_route) | (i2c_setup->sda_pin_route);
  i2c->ROUTEPEN = (i2c_setup->scl_pin_en * I2C_ROUTEPEN_SCLPEN) |
                  (i2c_setup->sda_pin_en * I2C_ROUTEPEN_SDAPEN);

  i2c->IFC = I2C_IEN_MSTOP | I2C_IEN_ACK | I2C_IEN_RXDATAV;
  i2c->IEN = I2C_IEN_MSTOP | I2C_IEN_ACK | I2C_IEN_RXDATAV;
  if (i2c == I2C0) NVIC_EnableIRQ(I2C0_IRQn);
  else NVIC_EnableIRQ(I2C1_IRQn);
}

/***************************************************************************//**
 * @brief
 *  starts up i2c communication, setup for either burst read or burst write
 *
 * @details
 *  starts up i2c communication with the input parameters. the function is currently
 *  setup for the burst read or burst write command.
 *
 * @param[in] i2c
 *    the specific i2c bus to be used in communication
 *
 * @param[in] RW, bytesRequested, data
 *  RW:
 *    whether the command is burst read or burst write
 *  bytesRequested:
 *    the amount of bytes needing to be received or requested
 *  data:
 *    pointer the data buffer to be wrote to or read from
 *
 *@param[in] address, regAddress
 *  address:
 *   the i2c address of the peripheral requesting use of the i2c bus
 *  regAddress:
 *    the register address to be read from or wrote to
 *
 *@param[in] CB
 *    the call back function to be serviced after completion of transfer
 *
 * @note
 * This function has only been tested in burst read configuration as of Lab4
 * completion.
 *
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, bool RW, int bytesRequested, uint32_t address, uint32_t regAddress, uint32_t * data, int CB)
{
  if (i2c == I2C0)
  {
      while(stateMachine0.busy);
      EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) ==
          I2C_STATE_STATE_IDLE);
      stateMachine0.busy = true;

      stateMachine0.RW = RW;
      stateMachine0.i2c = i2c;
      stateMachine0.bytesDone = 0;
      stateMachine0.bytesRequested = bytesRequested;
      stateMachine0.state = START_WRITE;
      stateMachine0.regAddress = regAddress;
      stateMachine0.address = address << 1;
      stateMachine0.data = data;
      stateMachine0.CB = CB;
  }
  else
  {
      while(stateMachine1.busy);
      EFM_ASSERT((i2c->STATE & _I2C_STATE_STATE_MASK) ==
          I2C_STATE_STATE_IDLE);
      stateMachine1.busy = true;

      stateMachine1.RW = RW;
      stateMachine1.i2c = i2c;
      stateMachine1.bytesDone = 0;
      stateMachine1.bytesRequested = bytesRequested;
      stateMachine1.state = START_WRITE;
      stateMachine1.regAddress = regAddress;
      stateMachine1.address = address << 1;
      stateMachine1.data = data;
      stateMachine1.CB = CB;
  }

  sleep_block_mode(I2C_EM_BLOCK);
  i2c_bus_reset(i2c);
  i2c->CMD = I2C_CMD_START;
  i2c->TXDATA = address << 1;
}


/***************************************************************************//**
 * @brief
 * services ack interrupts
 *
 * @details
 * function called by the IRQ handler, to be used in moving along the state machine
 * when acks occur
 *
 * @param[in] stateMachine
 *  The stateMachine to be evaluated, I2C0 handler calls with stateMachine0,
 *  I2C1 with stateMachine1
 *
 * @note
 *  called by the IRQ handlers, only tested for burst read, should still work with
 *  write
 *
 *
 ******************************************************************************/
static void i2c_ack(I2C_STATE_MACHINE *stateMachine)
{
  switch (stateMachine->state)
  {
    case START_WRITE:
      stateMachine->state = SEND_REG;
      stateMachine->i2c->TXDATA = stateMachine->regAddress;
      break;
    case SEND_REG:
      if (stateMachine->RW == R)
      {
          stateMachine->state = START_READ;
          stateMachine->i2c->CMD = I2C_CMD_START;
          stateMachine->i2c->TXDATA = stateMachine->address | R;
      }
      else
      {
          stateMachine->state = WRITE_DATA;
          stateMachine->i2c->TXDATA = *(stateMachine->data++);
          stateMachine->bytesDone++;
      }
      break;
    case START_READ:
      if (stateMachine->RW == W) EFM_ASSERT(false);
      stateMachine->state = READ_DATA;
      break;
    case WRITE_DATA:
      if (stateMachine->RW == R) EFM_ASSERT(false);
      if (stateMachine->bytesDone == stateMachine->bytesRequested)
      {
          stateMachine->state = SEND_STOP;
          stateMachine->i2c->CMD = I2C_CMD_STOP;
      }
      else
      {
          stateMachine->i2c->TXDATA = *(stateMachine->data++);
          stateMachine->bytesDone++;
      }
      break;
    case READ_DATA:
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * function to service RXDATAV interrupts
 *
 * @details
 * called in the cases data is on the rxbuffer, should only happen in case of
 * burst read
 *
 * @param[in] stateMachine
 *  The stateMachine to be evaluated, I2C0 handler calls with stateMachine0,
 *  I2C1 with stateMachine1
 *
 * @note
 *  called by irq handlers, only used in burst read
 *
 *
 ******************************************************************************/
static void i2c_rxdata(I2C_STATE_MACHINE *stateMachine)
{
  switch(stateMachine->state)
  {
    case READ_DATA:
      if (stateMachine->RW == W) EFM_ASSERT(false);
      *(stateMachine->data++) = stateMachine->i2c->RXDATA;
      stateMachine->bytesDone++;

      if (stateMachine->bytesDone == stateMachine->bytesRequested)
      {
          stateMachine->state = SEND_STOP;
          stateMachine->i2c->CMD = I2C_CMD_NACK;
          stateMachine->i2c->CMD = I2C_CMD_STOP;
      }
      else stateMachine->i2c->CMD = I2C_CMD_ACK;
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * function to service MSTOP interrupts
 *
 * @details
 * called in the cases master terminates communication with stop command, allows
 * the stateMachine to be used again and schedules the call back
 *
 * @param[in] stateMachine
 *  The stateMachine to be evaluated, I2C0 handler calls with stateMachine0,
 *  I2C1 with stateMachine1
 *
 * @note
 *  called by irq handlers, signals end of communication
 *
 *
 ******************************************************************************/
static void i2c_mstop(I2C_STATE_MACHINE *stateMachine)
{
  switch(stateMachine->state)
  {
    case SEND_STOP:
      stateMachine->busy = false;
      stateMachine->state = DONE_STATE;
      sleep_unblock_mode(I2C_EM_BLOCK);
      add_scheduled_event(stateMachine->CB);
      break;
    default:
      EFM_ASSERT(false);
      break;
  }
}

/***************************************************************************//**
 * @brief
 * IRQ handler for I2C1
 *
 * @details
 * checks the interrupt flags and calls the correct helper function to service
 * the incoming interrupts
 *
 * @note
 *  one of two handlers, allows for use of both i2c buses simultaneously.
 *
 *
 ******************************************************************************/
void I2C1_IRQHandler(void)
{
  uint32_t intFlag = I2C1->IEN & I2C1->IF;
  uint32_t savedIEN = I2C1->IEN;
  I2C1->IFC = intFlag;
  I2C1->IEN &= ~I2C1->IEN;

  if (intFlag & I2C_IF_ACK)
  {
      i2c_ack(&stateMachine1);
  }
  if (intFlag & I2C_IF_RXDATAV)
  {
      i2c_rxdata(&stateMachine1);
  }
  if (intFlag & I2C_IF_MSTOP)
  {
      i2c_mstop(&stateMachine1);
  }

  I2C1->IEN = savedIEN;
}

/***************************************************************************//**
 * @brief
 * IRQ handler for I2C0
 *
 * @details
 * checks the interrupt flags and calls the correct helper function to service
 * the incoming interrupts
 *
 * @note
 *  one of two handlers, allows for use of both i2c buses simultaneously.
 *
 *
 ******************************************************************************/
void I2C0_IRQHandler(void)
{
  uint32_t intFlag = I2C0->IEN & I2C0->IF;
  uint32_t savedIEN = I2C0->IEN;
  I2C0->IFC = intFlag;
  I2C0->IEN &= ~I2C0->IEN;

  if (intFlag & I2C_IF_ACK)
  {
      i2c_ack(&stateMachine0);
  }
  if (intFlag & I2C_IF_RXDATAV)
  {
      i2c_rxdata(&stateMachine0);
  }
  if (intFlag & I2C_IF_MSTOP)
  {
      i2c_mstop(&stateMachine0);
  }

  I2C0->IEN = savedIEN;
}
