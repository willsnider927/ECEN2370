/*
 * sleep_routines.h
 *
 *  Created on: Sep 21, 2021
 *      Author: will
 */
/***************************************************************************
 * @file sleep.c
 ***************************************************************************
 * @section License*
 * <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 * *************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
 * obligation to support this Software. Silicon Labs is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitnessfor any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Silicon Labs will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 * ***************************************************************************/
//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"

//***********************************************************************************
// Private variables
//***********************************************************************************
static int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 * Initializes the energy mode static array
 *
 * @details
 * Initializes the energy mode static array to all zeros
 * signifying that any energy mode is available
 *
 * @note
 * Call with all other init functions at the start of the program
 *
 ******************************************************************************/
void sleep_open(void)
{
  for (int i = 0; i < MAX_ENERGY_MODES; i++)
  {
      lowest_energy_mode[i] = 0;
  }
}

/***************************************************************************//**
 * @brief
 * Blocks an energy mode from being entered
 *
 * @details
 * Updates the energy mode static array to increment the correct element
 * to signify that a specific peripheral cannot enter that energy state
 *
 * @note
 * Critical operation, cannot be updated
 *
 * @param[in] EM
 * The energy mode that cannot be entered
 *
 ******************************************************************************/
void sleep_block_mode(uint32_t EM)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM]++;
  EFM_ASSERT(lowest_energy_mode[EM] < 5);

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * Unblocks an energy mode from being entered
 *
 * @details
 * Updates the energy mode static array to decrement the correct element
 * to signify that a specific peripheral can now enter that energy state
 *
 * @note
 * Critical operation, cannot be updated
 *
 * @param[in] EM
 * The energy mode that can now be entered by a peripheral
 *
 ******************************************************************************/
void sleep_unblock_mode(uint32_t EM)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  lowest_energy_mode[EM]--;
  EFM_ASSERT(lowest_energy_mode[EM] >= 0);

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * Enters the highest energy mode available to all peripherals
 *
 * @details
 * Enters the highest energy mode (least energy intensive) that won't
 * complicate required peripheral function
 *
 * @note
 * Critical operation, cannot be updated
 *
 ******************************************************************************/
void enter_sleep(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  if (lowest_energy_mode[EM0] > 0){}
  else if (lowest_energy_mode[EM1] > 0){}
  else if (lowest_energy_mode[EM2] > 0) EMU_EnterEM1();
  else if (lowest_energy_mode[EM3] > 0) EMU_EnterEM2(true);
  else EMU_EnterEM3(true);

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 * returns the highest energy mode currently blocked
 *
 * @details
 * Increments through the energy mode static array until the first
 * not available energy mode is found
 *
 * @param[out] i/energy_mode
 * The energy mode that cannot be entered
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void)
{
  for (int i = 0; i < MAX_ENERGY_MODES; i++)
  {
      if (lowest_energy_mode[i] != 0) return i;
  }
  return MAX_ENERGY_MODES - 1;
}




