 /**
 * @file scheduler.c
 * @author Will Snider
 * @date 14 Sept 2021
 * @brief Scheduler
 *
 */

//***********************************************************************************
// Include files
//***********************************************************************************
#include "scheduler.h"
#include "em_assert.h"
#include "em_core.h"
#include "em_emu.h"
//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static unsigned int event_scheduled;

//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 *   Function to initialize the scheduler.
 *
 * @details
 *   Initialize the private variable that stores events to all 0s (no events).
 *
 ******************************************************************************/
void scheduler_open(void) { event_scheduled = 0; }

/***************************************************************************//**
 * @brief
 *   Adds scheduled events.
 *
 * @details
 *   Adds a 1 to the bit position that corresponds to the event to be scheduled.
 *
 * @note
 *   This function creates an atomic operation so interrupts cannot interfere with the scheduler.
 *
 * @param[in] event
 *   Use macros defined in app.h to pass in events to schedule
 *
 ******************************************************************************/
void add_scheduled_event(uint32_t event)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled |= event;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Removes scheduled events.
 *
 * @details
 *   Sets a 0 to the bit position that corresponds to the event to be removed from the schedule.
 *
 * @note
 *   This function creates an atomic operation so interrupts cannot interfere with the scheduler.
 *
 * @param[in] event
 *   Use macros defined in app.h to pass in events to remove from the schedule.
 *
 ******************************************************************************/
void remove_scheduled_event(uint32_t event)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  event_scheduled &= ~event;
  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Returns the scheduled events.
 *
 * @details
 *   Returns the private variable holding bits corresponding to scheduled events.
 *
 * @param[out] event_scheduled
 *   Private variable signifying scheduled events.
 *
 ******************************************************************************/
uint32_t get_scheduled_events(void) { return event_scheduled; }
