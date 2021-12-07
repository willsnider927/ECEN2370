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


//***********************************************************************************
// defined files
//***********************************************************************************
#define   PWM_PER         2.0   // PWM period in seconds
#define   PWM_ACT_PER     0.002  // PWM active period in seconds

// Application Schedule Events
#define   LETIMER0_COMP0_CB     0x00000001  // 0b0001
#define   LETIMER0_COMP1_CB     0x00000002  // 0b0010
#define   LETIMER0_UF_CB        0x00000004  // 0b0100
#define   Si1133_LIGHT_READ_CB  0x00000008
#define   BOOT_UP_CB            0x00000010
#define   BLE_TX_DONE_CB        0x00000020

#define   SYSTEM_BLOCK_EM       EM3
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
void scheduled_Si1133_light_read_cb(void);
void scheduled_boot_up_cb(void);
void scheduled_ble_tx_done_cb(void);

#endif
