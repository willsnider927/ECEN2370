//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef SLEEP_ROUTINES_HG
#define SLEEP_ROUTINES_HG

/* System include statements */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* Silicon Labs include statements */
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_assert.h"
#include "em_core.h"


/* The developer's include statements */
//#include "app.h"
//#include "brd_config.h"
//#include "scheduler.h"

//***********************************************************************************
// defined files
//***********************************************************************************



//***********************************************************************************
// global variables
//***********************************************************************************
#define EM0               0
#define EM1               1
#define EM2               2
#define EM3               3
#define EM4               4
#define MAX_ENERGY_MODES  5

//***********************************************************************************
// function prototypes
//***********************************************************************************
void sleep_open(void);
void sleep_block_mode(uint32_t EM);
void sleep_unblock_mode(uint32_t EM);
void enter_sleep(void);
uint32_t current_block_energy_mode(void);

#endif
