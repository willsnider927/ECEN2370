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
static uint32_t si1133_write_data;

//***********************************************************************************
// Private functions
//***********************************************************************************
static void si1133_configure(void);

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

  si1133_configure();
}
/***************************************************************************//**
 * @brief
 * starts a read of the specified register
 *
 * @details
 * calls the i2c_start function with all the data needed to start a read of
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
 ******************************************************************************/
void Si1133_read(uint32_t regAddress, uint32_t CB, int size)
{
  i2c_start(I2C1, R, size, SI1133_ADDRESS,regAddress, &si1133_read_result, CB);
}

/***************************************************************************//**
 * @brief
 * starts a write to the specified register
 *
 * @details
 * calls the i2c_start function with all the data needed to start a write
 * the specified register
 *
 * @param[in] regAddress, CB, size
 *  regAddress:
 *    the register to send to
 *  CB:
 *    the Call back function that will be called upon completion
 *  size:
 *    the expected size of the data to be sent in bytes
 *
 ******************************************************************************/
void Si1133_write(uint32_t regAddress, uint32_t CB, int size)
{
  i2c_start(I2C1, W, size, SI1133_ADDRESS, regAddress, &si1133_write_data, CB);
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

/***************************************************************************//**
 * @brief
 *  Configures the si1133 to be ready for force read
 *
 * @details
 *  sends and retrieves a series of packages through i2c to correctly configure
 *  the si1133 for use with it's white sensing diode. Confirms successful commands
 *  through checking the si1133's command counter.
 *
 * @note
 *  private function called within the file by the initialization function
 ******************************************************************************/
static void si1133_configure(void)
{
  uint32_t commandCtr = 0;
  uint32_t commandCtr1 = 0;
  si1133_read_result = 0;

  Si1133_read(SI1133_RESPONSE0_REG, NULL_CB, 1);
  while (isSM1Busy());
  commandCtr = si1133_pass_read() & 0x0F; //masking off command counter field

  si1133_write_data = WHITE_PHOTODIODE;
  Si1133_write(SI1133_INPUT0_REG, NULL_CB, 1);
  while(isSM1Busy());

  si1133_write_data = SI1133_CONFIG0_PARAM | SI1133_PARAM_WRITE;
  Si1133_write(SI1133_COMMAND_REG, NULL_CB, 1);
  while(isSM1Busy());

  Si1133_read(SI1133_RESPONSE0_REG, NULL_CB, 1);
  while(isSM1Busy());
  commandCtr1 = si1133_pass_read() & 0x0F; // mask off needed bits
  if(commandCtr1 != ((commandCtr+1) & 0x0F)) EFM_ASSERT(false); //command failed

  si1133_write_data = CHANNEL0_ACTIVE;
  Si1133_write(SI1133_INPUT0_REG, NULL_CB, 1);
  while(isSM1Busy());

  si1133_write_data = PARAM_CHAN_LIST | SI1133_PARAM_WRITE;
  Si1133_write(SI1133_COMMAND_REG, NULL_CB, 1);
  while(isSM1Busy());
  si1133_write_data = 0;

  Si1133_read(SI1133_RESPONSE0_REG, NULL_CB, 1);
  while(isSM1Busy());
  commandCtr1 = si1133_pass_read() & 0x0F; //mask off ctr bits
  if(commandCtr1 != ((commandCtr+2) & 0x0F)) EFM_ASSERT(false); //cmd failed
}
/***************************************************************************//**
 * @brief
 *  sends a force command to the si1133
 *
 * @details
 * writes to the SI1133 command register the force command, which forces a reading
 * using the settings in the configuration function
 *
 * @note
 *  called by the comp1 call back function in this application
 *
 ******************************************************************************/
void Si1133_force(void)
{
  si1133_write_data = FORCE_COMMAND;
  Si1133_write(SI1133_COMMAND_REG, NULL_CB, 1);
  while(isSM1Busy());
  si1133_write_data = 0;
}
/***************************************************************************//**
 * @brief
 *  Reads the result of the forced measurement
 *
 * @details
 *  reads the si1133's hostout1 register for the result of the measurement
 *
 * @note
 * Called by the letimer uf callback, required to be called at least 48uS after
 * the force read for the white photodiode
 ******************************************************************************/
void Si1133_ret_force(uint32_t CB)
{
  Si1133_read(SI1133_HOSTOUT1,CB,1);
}
