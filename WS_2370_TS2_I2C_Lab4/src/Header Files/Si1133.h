/*
 * Si1133.h
 *
 *  Created on: Oct 5, 2021
 *      Author: will
 */

#ifndef HEADER_FILES_SI1133_H_
#define HEADER_FILES_SI1133_H_

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_assert.h"
#include "em_i2c.h"

/* The developer's include statements */
#include "brd_config.h"
#include "i2c.h"
#include "HW_delay.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************
#define SI1133_ADDRESS 0x55
#define SI1133_PARTID  0x33
#define SI1133_ID_REG  0x00
//***********************************************************************************
// function prototypes
//***********************************************************************************
void Si1133_i2c_open();
void Si1133_read(uint32_t regAddress,uint32_t CB, int size);
uint32_t si1133_pass_read();


#endif /* HEADER_FILES_SI1133_H_ */
