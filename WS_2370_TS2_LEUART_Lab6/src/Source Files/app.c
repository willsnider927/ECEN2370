 /**
 * @file app.c
 * @author Will Snider
 * @date 7 Sept 2021
 * @brief Configures all peripherals
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "app.h"


//***********************************************************************************
// defined files
//***********************************************************************************
//#define BLE_TEST_ENABLED

//***********************************************************************************
// Private variables
//***********************************************************************************
static int LED_COLOR;
static uint32_t x = 3, y = 0;

//***********************************************************************************
// Private functions
//***********************************************************************************

static void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route);

//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Initializes all peripherals
 *
 * @details
 * This function calls low-level drivers to setup the peripherals. It enables and routes the clock to them.
 *
 * @note
 * cmu_open() must be called first. PWM period and active period can be configured as a define in app.h. PWM routing can be configured in brd_config.h.
 *
 ******************************************************************************/
void app_peripheral_setup(void){
  LED_COLOR = 0;
  sleep_block_mode(SYSTEM_BLOCK_EM);
  cmu_open();
  gpio_open();
  app_letimer_pwm_open(PWM_PER, PWM_ACT_PER, PWM_ROUTE_0, PWM_ROUTE_1);
  scheduler_open();
  sleep_open();
  rgb_init();
  Si1133_i2c_open();
  ble_open(BLE_TX_DONE_CB, NULL_CB);
  add_scheduled_event(BOOT_UP_CB);
}

/***************************************************************************//**
 * @brief
 * This function sets up the LETIMER to create a PWM waveform.
 *
 * @details
 * PWM specifications are stored in a struct (APP_LETIMER_PWM_TypeDef) which gets passed into the letimer_pwm_open function.
 *
 * @note
 * PWM is set by default to be idle LOW. This function does not start the PWM waveform.
 *
 * @param[in] period, act_period
 * Period is the full period of the PWM waveform during both active and idle sections. Act_period is the time that the PWM waveform will be active (HIGH) for.
 *
 * @param[in] out0_route, out1_route
 * out0_route, out1_route are the routes from LETIMER to the LED.
 ******************************************************************************/
void app_letimer_pwm_open(float period, float act_period, uint32_t out0_route, uint32_t out1_route){
  // Initializing LETIMER0 for PWM operation by creating the
  // letimer_pwm_struct and initializing all of its elements
  // APP_LETIMER_PWM_TypeDef is defined in letimer.h
  APP_LETIMER_PWM_TypeDef   letimer_pwm_struct;

  letimer_pwm_struct.debugRun = false;
  letimer_pwm_struct.enable = false;
  letimer_pwm_struct.period = period;
  letimer_pwm_struct.active_period = act_period;
  letimer_pwm_struct.out_pin_route0 = out0_route;
  letimer_pwm_struct.out_pin_route1 = out1_route;
  letimer_pwm_struct.out_pin_0_en = false;
  letimer_pwm_struct.out_pin_1_en = false;

  letimer_pwm_struct.uf_irq_enable = true;
  letimer_pwm_struct.comp1_irq_enable = true;
  letimer_pwm_struct.comp0_irq_enable = false;
  letimer_pwm_struct.comp0_cb = LETIMER0_COMP0_CB;
  letimer_pwm_struct.comp1_cb = LETIMER0_COMP1_CB;
  letimer_pwm_struct.uf_cb = LETIMER0_UF_CB;


  letimer_pwm_open(LETIMER0, &letimer_pwm_struct);
}

/***************************************************************************//**
 * @brief
 * schedule handler for si1133 light read
 *
 * @details
 * turns on a green led if the part ID read was successful, turns on a red led
 * otherwise. Sends data over uart connected BLE module
 *
 * @note
 * This function is application specific and can be changed depending on
 * needs of program
 ******************************************************************************/
void scheduled_Si1133_light_read_cb()
{
  EFM_ASSERT(!(get_scheduled_events() & Si1133_LIGHT_READ_CB));
  int read = Si1133_read_result();
  char data[32];
  if(read < 20)
  {
      leds_enabled(RGB_LED_1, COLOR_BLUE, true);
      sprintf(data,"it's dark, %d\n", read);
      ble_write(data);
  }
  else
  {
      leds_enabled(RGB_LED_1, COLOR_BLUE, false);
      sprintf(data,"it's light outside, %d\n",read);
      ble_write(data);
  }
}

/***************************************************************************//**
 * @brief
 * Callback for LETIMER0 underflow interrupt
 *
 * @details
 * reads value from forced read of the SI1133, also sends calculations over bluetooth
 *
 * @note
 * Application specific
 *
 ******************************************************************************/
void scheduled_letimer0_uf_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_UF_CB));
  float z;
  x += 3;
  y++;
  z = (float)x/y;
  char data[16];
  sprintf(data,"z = %.1f\n",z);
  ble_write(data);
  Si1133_force_return(Si1133_LIGHT_READ_CB);
}

/***************************************************************************//**
 * @brief
 * Callback function for LETIMER0 COMP0 interrupt
 *
 * @details
 * Unused
 *
 * @note
 * Unused
 *
 ******************************************************************************/
void scheduled_letimer0_comp0_cb(void)
{
  EFM_ASSERT(false);
}

/***************************************************************************//**
 * @brief
 * Callback function for LETIMER0 COMP1 interrupt.
 *
 * @details
 * Forces a read of the SI1133 sensor
 *
 * @note
 * An EFM_ASSERT statement ensures the event has been cleared from the scheduler.
 *
 ******************************************************************************/
void scheduled_letimer0_comp1_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & LETIMER0_COMP1_CB));
  Si1133_force();
}

/***************************************************************************//**
 * @brief
 * Callback function for booting up.
 *
 * @details
 * Either runs test of BLE module or writes "Hello World!" to bluetooth module
 * @note
 * An EFM_ASSERT statement ensures the event has been cleared from the scheduler.
 *
 ******************************************************************************/
void scheduled_boot_up_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & BOOT_UP_CB));
#ifdef BLE_TEST_ENABLED
  bool success = ble_test("Wills_BLE");
  EFM_ASSERT(success);
  timer_delay(2000);
#endif
  letimer_start(LETIMER0, true);  //This command will initiate the start of the LETIMER0
  ble_write("\nHello World!\n");
}

/***************************************************************************//**
 * @brief
 * Callback function for ble_tx interrupt.
 *
 * @details
 * UNUSED
 * @note
 * UNUSED
 ******************************************************************************/
void scheduled_ble_tx_done_cb(void)
{
  EFM_ASSERT(!(get_scheduled_events() & BLE_TX_DONE_CB));
}
