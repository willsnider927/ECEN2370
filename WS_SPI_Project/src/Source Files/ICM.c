/**
 * @file ICM.c
 * @author Will Snider
 * @date 16 Nov 2021
 * @brief ICM Driver
 *
 */
#include "ICM.h"

/***************************************************************************//**
 * @brief
 *  opens spi communication lines for peripheral
 *
 * @details
 * opens spi communication by setting the USART settings struct values to the peripherals
 * needs, passes into spi driver open function to complete setup
 *
 * @note
 *  called once by the app_peripheral_setup function
 ******************************************************************************/
void icm_spi_open(void)
{
  USART_OPEN_STRUCT spi_settings;

  spi_settings.autoCsEnable = false;
  spi_settings.autoCsHold = false;
  spi_settings.autoCsSetup = false;
  spi_settings.autoTx = false;
  spi_settings.baudRate = ICM_BAUD;
  spi_settings.clk_pin_en = true;
  spi_settings.clk_pin_route = ICM_CLK_ROUTE;
  spi_settings.clockmode = ICM_CLOCK_MODE;
  spi_settings.cs_pin_en = false;
  spi_settings.cs_pin_route = ICM_CS_ROUTE;
  spi_settings.dataBits = ICM_DATABITS;
  spi_settings.enable = usartEnable;
  spi_settings.master = true;
  spi_settings.miso_pin_en = true;
  spi_settings.miso_pin_route = ICM_MISO_ROUTE;
  spi_settings.mosi_pin_en = true;
  spi_settings.mosi_pin_route = ICM_MOSI_ROUTE;
  spi_settings.msbf = true;
  spi_settings.refFreq = ICM_REFFREQ;
  spi_settings.prsRxEnable = false;
  spi_settings.stopBits = ICM_STOPBITS;
  spi_settings.prsRxCh = 0;
  spi_settings.csPort = ICM_CS_PORT;
  spi_settings.csPin = ICM_CS_PIN;

  usart_open(ICM_USART,&spi_settings);
}

/***************************************************************************//**
 * @brief
 * configures and tests the ICM through SPI
 *
 * @details
 * Sets the ICM internal registers to needed values for correct measurements
 * Tests the writes by reading the value back and ensuring correct configuration
 * and functionality of SPI
 *
 * @param[out] success
 * returns whether the tests were successful.
 *
 * @note
 * called in BOOT_UP_CB where the result is asserted to be true
 * only chance for incorrect success result is if ICM registers are preset to correct
 * values and the read functionality works while write does not
 ******************************************************************************/
bool ICM_TDD_CONFIG(void)
{
  bool success = true;
  uint32_t cmd;

  cmd = PWRMGMT1_CFG;
  usart_start(ICM_USART, ICM_W, PWRMGMT1_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  timer_delay(1);

  usart_start(ICM_USART, ICM_R, PWRMGMT1_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  if(*ret_sm3_read() != (PWRMGMT1_CFG)) success = false;

  cmd = PWRMGMT2_CFG;
  usart_start(ICM_USART, ICM_W, PWRMGMT2_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  timer_delay(1);

  usart_start(ICM_USART, ICM_R, PWRMGMT2_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  if(*ret_sm3_read() != PWRMGMT2_CFG) success = false;

  cmd = LPCONFIG_CFG;
  usart_start(ICM_USART, ICM_W, LPCONFIG_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  timer_delay(1);

  usart_start(ICM_USART, ICM_R, LPCONFIG_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  if(*ret_sm3_read() != LPCONFIG_CFG) success = false;

  cmd = ACCEL_WOM_THR_CFG;
  usart_start(ICM_USART, ICM_W, ACCEL_WOM_THR_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  timer_delay(1);

  usart_start(ICM_USART, ICM_R, ACCEL_WOM_THR_REG, &cmd, NULL_CB, 1);
  while (isSm3Busy());
  if(*ret_sm3_read() != ACCEL_WOM_THR_CFG) success = false;

  return success;
}

/***************************************************************************//**
 * @brief
 *  starts ICM read
 *
 * @details
 * Starts ICM read by use of driver start function, passes in required values for a read
 * of the specified register
 *
 * @param[in] cb
 *  callback to be serviced after end of spi operation
 *
 * @param[in] reg
 *  the register to be read from
 *
 * @note
 *  ICM registers are 1 byte
 ******************************************************************************/
void ICM_read(int cb, uint32_t reg)
{
  usart_start(ICM_USART,ICM_R,reg,0x00,cb,1);
}

/***************************************************************************//**
 * @brief
 *  starts ICM write
 *
 * @details
 * Starts ICM write by use of driver start function, passes in required values for a write
 * to the specified register
 *
 * @param[in] cb
 *  callback to be serviced after end of spi operation
 *
 * @param[in] reg
 *  the register to be wrote to
 *
 * @param[in] val
 *  the value to be sent to the register
 *
 * @note
 *  ICM registers are 1 byte
 ******************************************************************************/
void ICM_write(int cb, uint32_t reg, uint32_t *val)
{
  usart_start(ICM_USART,ICM_W,reg,val,cb,1);
}

