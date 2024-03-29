//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef SCHEDULER_HG
#define	SCHEDULER_HG

/* System include statements */
#include <stdint.h>

/* Silicon Labs include statements */
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"

/* The developer's include statements */

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void scheduler_open(void);
void add_scheduled_event(uint32_t event);
void remove_scheduled_event(uint32_t event);
uint32_t get_scheduled_events(void);


#endif
