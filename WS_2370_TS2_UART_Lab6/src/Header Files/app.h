//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef APP_HG
#define APP_HG

/* System include statements */


/* Silicon Labs include statements */
#include "em_cmu.h"
#include "em_assert.h"

/* The developer's include statements */
#include "cmu.h"
#include "gpio.h"
#include "letimer.h"
#include "brd_config.h"
#include "scheduler.h"
#include "sleep_routines.h"
#include "LEDs_thunderboard.h"
#include "Si1133.h"
#include "ble.h"
#include "HW_delay.h"


//***********************************************************************************
// defined files
//***********************************************************************************
#define   PWM_PER         1.0   // PWM period in seconds
#define   PWM_ACT_PER     0.002  // PWM active period in seconds

//application scheduled events
#define LETIMER0_COMP0_CB    0x01
#define LETIMER0_COMP1_CB    0x02
#define LETIMER0_UF_CB       0x04
#define SI1133_LIGHT_READ_CB 0x08
#define BOOT_UP_CB           0x10

//System wide EM
#define SYSTEM_BLOCK_EM      EM3

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void app_peripheral_setup(void);
void scheduled_letimer0_uf_cb(void);
void scheduled_letimer0_comp0_cb(void);
void scheduled_letimer0_comp1_cb(void);
void scheduled_si1133_light_read_cb(void);
void scheduled_boot_up_cb(void);

#endif
