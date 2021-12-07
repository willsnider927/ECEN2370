/**
 * @file Si1133.c
 * @author Will Snider
 * @date 5 Oct 2021
 * @brief Si1133 Driver
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
static uint32_t si1133_write_data;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void Si1133_configure(void);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *  opens i2c communication lines for peripheral
 *
 * @details
 * opens i2c communication by creating an initialization struct with settings
 * specific to the Si1133. Uses I2C1
 *
 * @note
 *  called once by the app_peripheral_setup function
 ******************************************************************************/
void Si1133_i2c_open(void)
{
  timer_delay(Si1133_STARTUP_TIME);

  I2C_OPEN_STRUCT i2c_settings;
  i2c_settings.freq = I2C_FREQ_FAST_MAX;
  i2c_settings.clhr = i2cClockHLRAsymetric;
  i2c_settings.enable = true;
  i2c_settings.master = true;
  i2c_settings.scl_pin_route = SCL_ROUTE;
  i2c_settings.sda_pin_route = SDA_ROUTE;
  i2c_settings.refFreq = 0;
  i2c_settings.scl_pin_en = true;
  i2c_settings.sda_pin_en = true;

  i2c_open(I2Cx, &i2c_settings);
  Si1133_configure();
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
 *  size:
 *    the expected size of the data to be received in bytes
 *  CB:
 *    the call back function that will be called upon completion
 *
 * @note
 *  part ID should always be 0x33
 ******************************************************************************/
void Si1133_read(uint32_t reg, int size, int callback)
{
  i2c_start(I2Cx, size, Si1133_ADDRESS, reg, &si1133_read_result, R, callback);
}

/***************************************************************************//**
 * @brief
 * starts a write on the specified i2c bus
 *
 * @details
 * Initiates an i2c communication to the specified i2c bus to the specified
 * register. The callback function will be called when i2c is completed
 *
 * @param[in] regAddress, CB, size
 *  regAddress:
 *    the register to be read from
 *  size:
 *    the expected size of the data to be received in bytes
 *  CB:
 *    the call back function that will be called upon completion
 *
 * @note
 * Writes from the static variable si1133_write_data
 ******************************************************************************/
void Si1133_write(uint32_t reg, int size, int callback)
{
  i2c_start(I2Cx, size, Si1133_ADDRESS, reg, &si1133_write_data, W, callback);
}

/***************************************************************************//**
 * @brief
 * Configures the Si1133
 *
 * @details
 * Sets various parameters in the Si1133 to prepare for taking measurements. This
 * includes selecting the white photodiode for measurements.
 *
 * @note
 * If writing to registers is unsuccessful (command counter doesn't increase), program will enter EFM_ASSERT
 ******************************************************************************/
static void Si1133_configure()
{
  si1133_read_result = 0;
  Si1133_read(Si1133_RESPONSE0_REG, 1, NULL_CB);
  while(i2c_sm1_busy());
  uint32_t cmd_ctr = si1133_read_result & LOW4BIT_MASK; // mask lowest 4 bits

  si1133_write_data = Si1133_WHITEPHOTODIODE;
  Si1133_write(Si1133_INPUT0_REG, 1, NULL_CB);
  while(i2c_sm1_busy());

  si1133_write_data = (Si1133_CONFIG0_PARAM) | Si1133_PARAM_WRITE;
  Si1133_write(Si1133_COMMAND_REG, 1, NULL_CB);
  while(i2c_sm1_busy());

  Si1133_read(Si1133_RESPONSE0_REG, 1, NULL_CB);
  while(i2c_sm1_busy());
  uint32_t cmd_ctr2 = si1133_read_result & LOW4BIT_MASK;

  if(cmd_ctr2 != ((cmd_ctr+1)&LOW4BIT_MASK)) EFM_ASSERT(false);

  si1133_write_data = Si1133_CHANNEL0_SEL;
  Si1133_write(Si1133_INPUT0_REG, 1, NULL_CB);
  while(i2c_sm1_busy());

  si1133_write_data = Si1133_PARAM_WRITE | Si1133_CHAN_LIST;
  Si1133_write(Si1133_COMMAND_REG, 1, NULL_CB);
  while(i2c_sm1_busy());

  Si1133_read(Si1133_RESPONSE0_REG, 1, NULL_CB);
  while(i2c_sm1_busy());
  cmd_ctr = cmd_ctr2;
  cmd_ctr2 = si1133_read_result & LOW4BIT_MASK;

  if(cmd_ctr2 != ((cmd_ctr+1)&LOW4BIT_MASK)) EFM_ASSERT(false);
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
 ******************************************************************************/
uint32_t Si1133_read_result(void)
{
  uint32_t ret = si1133_read_result;
  si1133_read_result = 0;
  return ret;
}

/***************************************************************************//**
 * @brief
 * Commands the Si1133 to enter force mode
 *
 * @details
 * Force mode will force the Si1133 to take a measurement
 *
 * @note
 * Measurement data is read using the Si1133_force_return function
 ******************************************************************************/
void Si1133_force(void)
{
  si1133_write_data = Si1133_FORCE_COMMAND;
  Si1133_write(Si1133_COMMAND_REG, 1, NULL_CB);
  while(i2c_sm1_busy());
  si1133_write_data = 0;
}

/***************************************************************************//**
 * @brief
 * Reads the result of the Si1133 force command
 *
 * @details
 * Force mode will force the Si1133 to take a measurement
 *
 * @note
 * Measurement data is stored in the private variable si1133_read_result
 ******************************************************************************/
void Si1133_force_return(uint32_t callback)
{
  Si1133_read(Si1133_HOSTOUT1, 1, callback);
}
