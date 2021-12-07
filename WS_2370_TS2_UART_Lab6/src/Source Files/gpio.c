/**
 * @file gpio.c
 * @author Will Snider
 * @date 09/07/2021
 * @brief
 * Enables and initializes the GPIO pins.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************


//***********************************************************************************
// functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Function to open all required GPIO pins.
 *
 * @details
 * Function sets the strength and modes for the used LEDS and enables the CMU clocks
 * GPIO
 *
 * @note
 * CMU open is called before this function
 *
 ******************************************************************************/

void gpio_open(void){

  CMU_ClockEnable(cmuClock_GPIO, true);

	// Configure LED pins
	GPIO_DriveStrengthSet(LED_RED_PORT, LED_RED_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED_RED_PORT, LED_RED_PIN, LED_RED_GPIOMODE, LED_RED_DEFAULT);

	GPIO_DriveStrengthSet(LED_GREEN_PORT, LED_GREEN_DRIVE_STRENGTH);
	GPIO_PinModeSet(LED_GREEN_PORT, LED_GREEN_PIN, LED_GREEN_GPIOMODE, LED_GREEN_DEFAULT);

	GPIO_PinModeSet(RGB_ENABLE_PORT, RGB_ENABLE_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB0_PORT, RGB0_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB1_PORT, RGB1_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB2_PORT, RGB2_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB3_PORT, RGB3_PIN, gpioModePushPull, RGB_DEFAULT_OFF);
	GPIO_PinModeSet(RGB_RED_PORT, RGB_RED_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
	GPIO_PinModeSet(RGB_GREEN_PORT, RGB_GREEN_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);
	GPIO_PinModeSet(RGB_BLUE_PORT, RGB_BLUE_PIN, gpioModePushPull, COLOR_DEFAULT_OFF);

	GPIO_PinModeSet(SI1133_SENSOR_EN_PORT, SI1133_SENSOR_EN_PIN, gpioModePushPull, SI1133_DEFAULT_ON);
	GPIO_DriveStrengthSet(SI1133_SENSOR_EN_PORT, SI1133_DRIVESTRENGTH);
	GPIO_PinModeSet(SI1133_SCL_PORT, SI1133_SCL_PIN, gpioModeWiredAnd, 1);
	GPIO_PinModeSet(SI1133_SDA_PORT, SI1133_SDA_PIN, gpioModeWiredAnd, 1);

	GPIO_PinModeSet(LEUART_TX_PORT,LEUART_TX_PIN,gpioModePushPull,1);
	GPIO_DriveStrengthSet(LEUART_TX_PORT,LEUART_TX_DRIVE);
	GPIO_PinModeSet(LEUART_RX_PORT,LEUART_RX_PIN,gpioModeInput,1);
}
