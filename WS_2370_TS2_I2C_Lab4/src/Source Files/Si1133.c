/**
 * @file Si1133.c
 * @author Will Snider
 * @date 10/05/2021
 * @brief
 * file for managing si1133 i2c communication
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "Si1133.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static uint32_t si1133_read_result;

//***********************************************************************************
// Private functions
//***********************************************************************************

//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *  opens i2c communication lines for peripheral
 *
 * @details
 * opens i2c communication lines by creating the initialization struct with all the needed
 * settings for the peripheral, uses I2C1
 *
 * @note
 *  called once by the app peripheral init function
 *
 ******************************************************************************/
void Si1133_i2c_open()
{
  timer_delay(25);

  I2C_OPEN_STRUCT i2c_init;
  i2c_init.freq = I2C_FREQ_FAST_MAX;
  i2c_init.clhr = i2cClockHLRAsymetric;
  i2c_init.enable = true;
  i2c_init.master = true;
  i2c_init.scl_pin_route = SCL_ROUTE;
  i2c_init.sda_pin_route = SDA_ROUTE;
  i2c_init.sda_pin_en = true;
  i2c_init.scl_pin_en = true;
  i2c_init.refFreq = 0;
  i2c_open(I2C1, &i2c_init);
}
/***************************************************************************//**
 * @brief
 * starts a read of the specified register
 *
 * @details
 * calls the i2c burst read function with all the data needed to start a read of
 * the specified register
 *
 * @param[in] regAddress, CB, size
 *  regAddress:
 *    the register to be read from
 *  CB:
 *    the Call back function that will be called upon completion
 *  size:
 *    the expected size of the data to be received in bytes
 *
 * @note
 *  part ID should always be 0x33
 *
 ******************************************************************************/
void Si1133_read(uint32_t regAddress,uint32_t CB, int size)
{
  i2c_start(I2C1, R, size, SI1133_ADDRESS,regAddress, &si1133_read_result, CB);
}

/***************************************************************************//**
 * @brief
 *  returns the result of the read operation
 *
 * @details
 * resets the si1133_read_result static variable to 0 and returns the value
 * before reset
 *
 * @param[out] result
 *   the result of the read operation
 *
 * @note
 *  called by the call back function after successful communication.
 *
 ******************************************************************************/
uint32_t si1133_pass_read()
{
  uint32_t result = si1133_read_result;
  si1133_read_result = 0;
  return result;
}
