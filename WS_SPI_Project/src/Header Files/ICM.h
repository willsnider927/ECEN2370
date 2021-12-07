/*
 * ICM.h
 *
 *  Created on: Nov 9, 2021
 *      Author: will
 */

#ifndef HEADER_FILES_ICM_H_
#define HEADER_FILES_ICM_H_

/* System include statements */

/* Silicon Labs include statements */
#include "em_assert.h"
#include "em_usart.h"
#include "em_gpio.h"

/* The developer's include statements */
#include "brd_config.h"
#include "spi.h"
#include "HW_delay.h"

//defines
#define ICM_W                   0
#define ICM_R                   0x80

#define PWRMGMT1_CFG        (1<<5) | (1<<3)
#define PWRMGMT2_CFG        0x07
#define LPCONFIG_CFG        (0x01<<5)
#define ACCEL_WOM_THR_CFG   60

#define NULL_CB             0x00
//***********************************************************************************
// function prototypes
//***********************************************************************************
void icm_spi_open(void);
bool ICM_TDD_CONFIG(void);
void ICM_read(int cb, uint32_t reg);
void ICM_write(int cb, uint32_t reg, uint32_t *val);

#endif /* HEADER_FILES_ICM_H_ */
