/*
 * i2c.h
 *
 *  Created on: Sep 30, 2021
 *      Author: matt
 */

#ifndef I2C_H_
#define I2C_H_

//***********************************************************************************
// Include files
//***********************************************************************************
/* System include statements */
#include "em_i2c.h"
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

#define I2C_EM_BLOCK EM2
#define R 1   // read/write bits for I2C
#define W 0

//***********************************************************************************
// global variables
//***********************************************************************************
typedef enum
{
  START_WRITE,
  SEND_REG,
  START_READ,
  READ_DATA,
  DONE_STATE,
  WRITE_DATA,
  SEND_STOP
} DEFINED_STATES;

typedef struct {
  bool                  enable;         // Enable I2C peripheral when initialization completed
  bool                  master;         // Set to master (true) or slave (false) mode
  uint32_t              refFreq;        // I2C reference clock assumed when configuring bus frequency setup
  uint32_t              freq;           // (Max) I2C bus frequency to use
  I2C_ClockHLR_TypeDef  clhr;           // Clock low/high ratio control
  uint32_t              sda_pin_route;
  uint32_t              scl_pin_route;
  bool                  sda_pin_en;
  bool                  scl_pin_en;
} I2C_OPEN_STRUCT;

typedef struct {
  I2C_TypeDef           *i2c;
  DEFINED_STATES        state;
  int                   bytesRequested;
  int                   bytesDone;
  uint32_t              slaveAddress;
  uint32_t              regAddress;
  uint32_t              *data;
  int                   RW;
  volatile bool         busy;
  uint32_t              cb;
} I2C_STATE_MACHINE;

//***********************************************************************************
// function prototypes
//***********************************************************************************

void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void i2c_start(I2C_TypeDef *i2c, int bytesRequested, uint32_t address, uint32_t regAddress, uint32_t *data, int RW, int cb);
bool i2c_sm1_busy(void);


void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);

#endif /* I2C_H_ */
