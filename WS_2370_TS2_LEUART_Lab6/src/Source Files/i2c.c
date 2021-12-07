/**
 * @file i2c.c
 * @author Will Snider
 * @date 30 Sept 2021
 * @brief I2C Driver
 *
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
static I2C_STATE_MACHINE state_machine0;
static I2C_STATE_MACHINE state_machine1;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void i2c_ack(I2C_STATE_MACHINE *sm);
static void i2c_mstop(I2C_STATE_MACHINE *sm);
static void i2c_rxdata(I2C_STATE_MACHINE *sm);

/***************************************************************************//**
 * @brief
 *  resets the i2c bus of the specified input
 *
 * @details
 *  resets the i2c bus of I2C0 or I2C1
 *
 * @param[in] i2c
 *  the i2c bus to reset
 *
 * @note
 *  this function should be called between every i2c transfer
 ******************************************************************************/
static void i2c_bus_reset(I2C_TypeDef *i2c)
{
  uint32_t savedInt;

  i2c->CMD = I2C_CMD_ABORT;
  savedInt = i2c->IEN;
  i2c->IEN &= ~i2c->IEN;
  i2c->IFC = i2c->IF;
  i2c->CMD = I2C_CMD_CLEARTX;

  i2c->CMD = (I2C_CMD_START | I2C_CMD_STOP);
  while(!(i2c->IF & I2C_IF_MSTOP));
  i2c->IFC = i2c->IF;
  i2c->CMD = I2C_CMD_ABORT;
  i2c->IEN = savedInt;
}

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * initializes an i2c bus
 *
 * @details
 * initializes the i2c bus, enables routing of i2c and clocks;
 * sets specific i2c settings needed for the peripheral requesting initialization
 *
 * @param[in] i2c
 *    The bus being initialized, I2C0 or I2C1
 *
 * @param[in] i2c_setup
 *    The struct containing all the information needed to initialize the bus
 *
 * @note
 * This function is called by the peripheral, only two i2c peripherals are available.
 * Settings are specific to the peripheral
 ******************************************************************************/
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup)
{
  if(i2c == I2C0) CMU_ClockEnable(cmuClock_I2C0, true);
  else CMU_ClockEnable(cmuClock_I2C1, true);

  if ((i2c->IF & 0x01) == 0)
  {
      i2c->IFS = 0x01;
      EFM_ASSERT(i2c->IF & 0x01);
      i2c->IFC = 0x01;
  } else
  {
      i2c->IFC = 0x01;
      EFM_ASSERT(!(i2c->IF & 0x01));
  }

  I2C_Init_TypeDef i2c_values;

  i2c_values.master   = i2c_setup->master;
  i2c_values.enable   = i2c_setup->enable;
  i2c_values.freq     = i2c_setup->freq;
  i2c_values.refFreq  = i2c_setup->refFreq;
  i2c_values.clhr     = i2c_setup->clhr;

  I2C_Init(i2c, &i2c_values);

  i2c->ROUTELOC0 = i2c_setup->scl_pin_route | i2c_setup->sda_pin_route;
  i2c->ROUTEPEN  = (i2c_setup->scl_pin_en * I2C_ROUTEPEN_SCLPEN) |
                   (i2c_setup->sda_pin_en * I2C_ROUTEPEN_SDAPEN);

  i2c->IFC = I2C_IEN_MSTOP | I2C_IEN_ACK | I2C_IEN_RXDATAV;
  i2c->IEN = I2C_IEN_MSTOP | I2C_IEN_ACK | I2C_IEN_RXDATAV;

  if (i2c == I2C0) NVIC_EnableIRQ(I2C0_IRQn);
  else NVIC_EnableIRQ(I2C1_IRQn);
  i2c_bus_reset(i2c);
}


/***************************************************************************//**
 * @brief
 *  starts up i2c communication
 *
 * @details
 *  starts up i2c communication with the input parameters. This function
 *  can be used for burst read or burst write
 *
 * @param[in] i2c
 *    the specific i2c bus to be used
 *
 * @param[in] bytesRequested
 *    the amount of bytes needing to be received or requested
 *
 * @param[in] address
 *   the i2c address of the peripheral to communicate to
 *
 * @param[in] regAddress
 *    the register address to read/write from
 *
 * @param[in] data
 *    pointer to the data buffer to read/write from
 *
 * @param[in] RW
 *    specifies burst read or burst write
 *
 * @param[in] CB
 *    the call back function to be serviced after completion of communication
 *
 * @note
 * This function has only been tested in burst read configuration as of Lab4
 * completion.
 ******************************************************************************/
void i2c_start(I2C_TypeDef *i2c, int bytesRequested, uint32_t address, uint32_t regAddress, uint32_t *data, int RW, int cb) // write = 0
{
  if(i2c == I2C0)
  {
    while(state_machine0.busy);
    EFM_ASSERT((I2C0->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);

    state_machine0.i2c = i2c;
    state_machine0.bytesRequested = bytesRequested;
    state_machine0.bytesDone = 0;
    state_machine0.slaveAddress = address;
    state_machine0.regAddress = regAddress;
    state_machine0.data = data;
    state_machine0.state = START_WRITE;
    state_machine0.busy = true;
    state_machine0.RW = RW;
    state_machine0.cb = cb;
  }
  if(i2c == I2C1)
  {
    while(state_machine1.busy);
    EFM_ASSERT((I2C1->STATE & _I2C_STATE_STATE_MASK) == I2C_STATE_STATE_IDLE);

    state_machine1.i2c = i2c;
    state_machine1.bytesRequested = bytesRequested;
    state_machine1.bytesDone = 0;
    state_machine1.slaveAddress = address;
    state_machine1.regAddress = regAddress;
    state_machine1.data = data;
    state_machine1.state = START_WRITE;
    state_machine1.busy = true;
    state_machine1.RW = RW;
    state_machine1.cb = cb;
  }
  sleep_block_mode(I2C_EM_BLOCK);
  i2c->CMD = I2C_CMD_START;
  i2c->TXDATA = ((address << 1) | W);
}

/***************************************************************************//**
 * @brief
 * Returns true if the state machine is busy
 *
 * @details
 * Returns the state of the state machine busy function

 * @note
 * This is useful for other files needing to know if the state machine is still
 * communicating with the slave.
 ******************************************************************************/
bool i2c_sm1_busy(void) { return state_machine1.busy; }

/***************************************************************************//**
 * @brief
 * services ack interrupts
 *
 * @details
 * this function is called by the IRQ handler, to be used in moving along the
 * state machine when ack interrupts occur
 *
 * @param[in] stateMachine
 *  The stateMachine to be evaluated: I2C0 handler calls with stateMachine0,
 *  I2C1 with stateMachine1
 *
 * @note
 *  called by the IRQ handlers, only tested for burst read, as of Lab4
 ******************************************************************************/
static void i2c_ack(I2C_STATE_MACHINE *sm)
{
  switch(sm->state)
  {
    case START_WRITE:
      sm->state = SEND_REG;
      sm->i2c->TXDATA = sm->regAddress;
      break;
    case SEND_REG:
      if(sm->RW == R)
      {
          sm->state = START_READ;
          sm->i2c->CMD = I2C_CMD_START;
          sm->i2c->TXDATA = ((sm->slaveAddress << 1) | R);
      } else if (sm->RW == W)
      {
          sm->state = WRITE_DATA;
          sm->i2c->TXDATA = *(sm->data++);
          sm->bytesDone++;
      }
      break;
    case START_READ:
      if(sm->RW == W) EFM_ASSERT(false);
      sm->state = READ_DATA;
      break;
    case WRITE_DATA:
      if(sm->RW == R) EFM_ASSERT(false);
      if(sm->bytesDone == sm->bytesRequested)
      {
          sm->state = SEND_STOP;
          sm->i2c->CMD = I2C_CMD_STOP;
      } else {
          sm->i2c->TXDATA = *(sm->data++);
          sm->bytesDone++;
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
 * called in the case that data is on the rxbuffer. This should only happen in
 * burst read
 *
 * @param[in] stateMachine
 *  The stateMachine to be evaluated: I2C0 handler calls with stateMachine0,
 *  I2C1 with stateMachine1
 *
 * @note
 *  called by IRQ handlers, only used in burst read
 ******************************************************************************/
static void i2c_rxdata(I2C_STATE_MACHINE *sm)
{
  switch (sm->state)
  {
    case READ_DATA:
      if(sm->RW == W) EFM_ASSERT(false);
      *(sm->data++) = sm->i2c->RXDATA;
      sm->bytesDone++;
      if(sm->bytesDone == sm->bytesRequested)
      {
          sm->state = SEND_STOP;
          sm->i2c->CMD = I2C_CMD_NACK;
          sm->i2c->CMD = I2C_CMD_STOP;
      } else {
          sm->i2c->CMD = I2C_CMD_ACK;
      }
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
 * called in the case that master terminates communication with stop command, allows
 * the stateMachine to be used again and schedules the call back
 *
 * @param[in] stateMachine
 *  The stateMachine to be evaluated: I2C0 handler calls with stateMachine0,
 *  I2C1 with stateMachine1
 *
 * @note
 *  called by IRQ handlers, signals end of communication
 ******************************************************************************/
static void i2c_mstop(I2C_STATE_MACHINE *sm)
{
  switch (sm->state)
  {
    case SEND_STOP:
      sm->busy = false;
      sm->state = DONE_STATE;
      sleep_unblock_mode(I2C_EM_BLOCK);
      add_scheduled_event(sm->cb);
      break;
    default:
       EFM_ASSERT(false);
       break;
  }
}


/***************************************************************************//**
 * @brief
 * IRQ handler for I2C0
 *
 * @details
 * checks the interrupt flags and calls the correct function to service
 * the incoming interrupts
 *
 * @note
 *  one of two handlers, allows for use of both i2c buses simultaneously.
 ******************************************************************************/
void I2C0_IRQHandler(void)
{
  uint32_t intFlags = I2C0->IF & I2C0->IEN;
  uint32_t savedInterrupts = I2C0->IEN;
  I2C0->IEN &= ~I2C0->IEN; // Disable all interrupts
  I2C0->IFC = intFlags;

  if(intFlags & I2C_IF_ACK)
  {
      i2c_ack(&state_machine0);
  }
  if(intFlags & I2C_IF_RXDATAV)
  {
      i2c_rxdata(&state_machine0);
  }
  if(intFlags & I2C_IF_MSTOP)
  {
      i2c_mstop(&state_machine0);
  }
  I2C0->IEN = savedInterrupts;
}


/***************************************************************************//**
 * @brief
 * IRQ handler for I2C1
 *
 * @details
 * checks the interrupt flags and calls the correct function to service
 * the incoming interrupts
 *
 * @note
 *  one of two handlers, allows for use of both i2c buses simultaneously.
 ******************************************************************************/
void I2C1_IRQHandler(void)
{
  uint32_t intFlags = I2C1->IF & I2C1->IEN;
  uint32_t savedInterrupts = I2C1->IEN;
  I2C1->IEN &= ~I2C1->IEN; // Disable all interrupts
  I2C1->IFC = intFlags;

  if(intFlags & I2C_IF_ACK)
  {
      i2c_ack(&state_machine1);
  }
  if(intFlags & I2C_IF_RXDATAV)
  {
      i2c_rxdata(&state_machine1);
  }
  if(intFlags & I2C_IF_MSTOP)
  {
      i2c_mstop(&state_machine1);
  }
  I2C1->IEN = savedInterrupts;
}
