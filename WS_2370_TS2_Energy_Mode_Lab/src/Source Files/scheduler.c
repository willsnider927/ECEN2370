/*
 * scheduler.c
 *
 *  Created on: Sep 14, 2021
 *      Author: will
 */

#include "scheduler.h"

//**********************************
// PRIVATE VARIABLES
//**********************************
static unsigned int event_scheduled;


/***************************************************************************//**
 * @brief
 * Opens the scheduler
 *
 * @details
 * Opens the scheduler by correctly setting the event_scheduled static variable
 *
 * @note
 * This function should be called with all other initialization functions.
 *
 ******************************************************************************/
void scheduler_open(void)
{
  event_scheduled = 0;
}

/***************************************************************************//**
 * @brief
 * Adds scheduled events to static variable
 *
 * @details
 * updates the static variable so that when checked by the while loop in main
 * the correct event is run
 *
 * @note
 * Critical operation, cannot be updated
 *
 * @param[in] event
 * The event to be scheduled
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
 * Removes scheduled events from static variable
 *
 * @details
 * updates the static variable so that when checked by the while loop in main
 * the event is not run twice
 *
 * @note
 * Critical operation, cannot be updated
 *
 * @param[in] event
 * The event already handled and to be removed from the scheduler
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
 * Returns the currently scheduled events
 *
 * @details
 * Returns the currently scheduled events so that the while loop in main
 * can correctly evaluate which functions to perform
 *
 * @param[out] event_scheduled
 * The events currently scheduled
 *
 ******************************************************************************/
uint32_t get_scheduled_events(void)
{
  return event_scheduled;
}
