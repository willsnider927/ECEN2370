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
  cmu_open();
  gpio_open();
  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  letimer_start(LETIMER0, true);  //This command will initiate the start of the LETIMER0
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
  letimer_pwm_struct.out_pin_0_en = true;
  letimer_pwm_struct.out_pin_1_en = true;
  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;

  letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}


