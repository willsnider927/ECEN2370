/**
 * @file app.c
 * @author Will Snider
 * @date 09/07/2021
 * @brief
 * Calls all the required functions to initialize the peripherals being used.
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static int LED_COLOR;

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Calls each of the individual open functions in the required order, calls
 * the function to start letimer.
 *
 * @details
 * Sets up each of the peripherals in the correct order, and lastly starts the LETIMER.
 * All the correct details for peripheral initialization are passed into functions that would need it.
 *
 * @note
 * Call this function rather than all the individual Init/Open functions
 *
 ******************************************************************************/

void app_peripheral_setup(void){
  LED_COLOR = 0;
  cmu_open();
  gpio_open();
  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  scheduler_open();
  sleep_open();
  rgb_init();
  Si1133_i2c_open();
  sleep_block_mode(SYSTEM_BLOCK_EM);
  ble_open(0x00,0x00);
  add_scheduled_event(BOOT_UP_CB);
}

/***************************************************************************//**
 * @brief
 * Initial setup before enabling the LETIMER0, creates a struct with all the information
 * the timer needs to start.
 *
 * @details
 * Does initial setup for the letimer by creating a struct that contains the information
 * needed to initialize the peripheral. All possibly variable operation details are passed in.
 *
 * @note
 * period, float, and the routing can be redefined for specific user needs outside of the function.
 *
 * @param[in] period, act_period
 * Defining of the period and active period for the timer, act_period is length output is HIGH.
 * period is the total period of time spent LOW + time spent HIGH.
 *
 * @param[in] out0_route, out1_route
 * Currently set up as routing for PD9 (Green LED), can be changed in brd_onfig.h
 *
 ******************************************************************************/

void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
  // Initializing LETIMER0 for PWM operation by creating the
  // letimer_pwm_struct and initializing all of its elements
  // APP_LETIMER_PWM_TypeDef is defined in letimer.h
  APP_LETIMER_PWM_TypeDef   letimer_pwm_struct;
  letimer_pwm_struct.enable = false;
  letimer_pwm_struct.period = period;
  letimer_pwm_struct.active_period = act_period;
  letimer_pwm_struct.debugRun = false;
  letimer_pwm_struct.out_pin_0_en = false;
  letimer_pwm_struct.out_pin_1_en = false;
  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;
  letimer_pwm_struct.comp1_irq_enable = true;
  letimer_pwm_struct.uf_irq_enable = true;
  letimer_pwm_struct.comp0_irq_enable = false;
  letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
  letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
  letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;

  letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}

/***************************************************************************//**
 * @brief
 * schedule handler for letimer0 underflow
 *
 * @details
 * reads the value of the forced measurement, passes the light read
 * call back to handle the result
 *
 * @note
 * This function is application specific and can be changed depending on
 * needs of program
 *
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_UF_CB));
  Si1133_ret_force(SI1133_LIGHT_READ_CB);
}

/***************************************************************************//**
 * @brief
 * Schedule handler for letimer0 comp0 interrupts
 *
 * @details
 * Does nothing in this application, should never be called
 *
 * @note
 * This function is application specific and can be changed depending on
 * needs of program
 *
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void)
{
  EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 * schedule handler for letimer0 comp1
 *
 * @details
 * calls the si1133 force function
 *
 * @note
 * This function is application specific and can be changed depending on
 * needs of program
 *
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_COMP1_CB));
  Si1133_force();
}

/***************************************************************************//**
 * @brief
 * schedule handler for si1133 light read
 *
 * @details
 *  turns on led if the light sensor does not detect light
 *
 * @note
 * This function is application specific and can be changed depending on
 * needs of program
 *
 *
 ******************************************************************************/
void scheduled_si1133_light_read_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & SI1133_LIGHT_READ_CB));
  uint32_t photo_read = si1133_pass_read();
  if (photo_read < 20)
  {
      leds_enabled(RGB_LED_1,COLOR_BLUE,true);
  }
  else
  {
      leds_enabled(RGB_LED_1,COLOR_BLUE,false);
  }
}

void scheduled_boot_up_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & BOOT_UP_CB));
  EFM_ASSERT(ble_test("Wills_BLE"));
  timer_delay(2000);
  letimer_start(LETIMER0, true);  //This command will initiate the start of the LETIMER
}

