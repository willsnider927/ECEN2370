/*
 * i2c.h
 *
 *  Created on: Sep 30, 2021
 *      Author: will
 */

#ifndef HEADER_FILES_I2C_H_
#define HEADER_FILES_I2C_H_
/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_assert.h"
#include "em_i2c.h"

/* The developer's include statements */
#include "brd_config.h"
#include "sleep_routines.h"
#include "scheduler.h"

//***********************************************************************************
// state machine enum
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
//***********************************************************************************
// global variables
//***********************************************************************************
typedef struct {
  I2C_ClockHLR_TypeDef clhr;
  bool           enable;
  uint32_t       freq;
  bool           master;
  uint32_t       refFreq;
  uint32_t       sda_pin_route;
  uint32_t       scl_pin_route;
  bool           sda_pin_en;
  bool           scl_pin_en;
} I2C_OPEN_STRUCT;

typedef struct {
  I2C_TypeDef *  i2c;
  DEFINED_STATES state;
  int            bytesRequested;
  int            bytesDone;
  uint32_t       regAddress;
  uint32_t       address;
  uint32_t       *data;
  volatile bool  busy;
  bool           RW;
  uint32_t       CB;
} I2C_STATE_MACHINE;
//***********************************************************************************
// defines
//***********************************************************************************
#define I2C_EM_BLOCK EM2
#define W  0
#define R  1

//***********************************************************************************
// function prototypes
//***********************************************************************************
void i2c_open(I2C_TypeDef *i2c, I2C_OPEN_STRUCT *i2c_setup);
void i2c_start(I2C_TypeDef * i2c, bool RW, int bytesRequested, uint32_t address, uint32_t regAddress, uint32_t * data, int CB);

#endif /* HEADER_FILES_I2C_H_ */
