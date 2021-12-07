 /**
 * @file sleep_routines.c
 * @author Will Snider
 * @date 21 Sept 2021
 * @brief Sleep Manager
 *
 */

/**************************************************************************
* * @file sleep.c
* ***************************************************************************
* * @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
* ***************************************************************************
* *
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions: *
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party. *
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
**************************************************************************/

//***********************************************************************************
// Include files
//***********************************************************************************
#include "sleep_routines.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************
static int lowest_energy_mode[MAX_ENERGY_MODES];

//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************
/***************************************************************************//**
 * @brief
 *   Enter the highest sleep mode possible.
 *
 * @details
 *   This function will check to see which sleep modes are blocked. It will
 *   then enter the highest unblocked sleep model
 *
 * @note
 *   This function uses atomic operations to ensure interrupts do not interfere.
 *
 ******************************************************************************/
void enter_sleep(void)
{
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  if(lowest_energy_mode[EM0] > 0) {}
  else if(lowest_energy_mode[EM1] > 0) {}
  else if(lowest_energy_mode[EM2] > 0) EMU_EnterEM1();
  else if(lowest_energy_mode[EM3] > 0) EMU_EnterEM2(true);
  else  EMU_EnterEM3(true);

  CORE_EXIT_CRITICAL();
}

/***************************************************************************//**
 * @brief
 *   Return the highest unblocked sleep mode.
 *
 * @details
 *   This function returns the highest unblocked sleep mode but doesn't enter it.
 *
 * @note
 *   This function does not enter sleep.
 *
 * @param[out] sleep_mode
 *   Highest unblocked sleep mode
 *
 ******************************************************************************/
uint32_t current_block_energy_mode(void)
{
  for(int i = 0; i < MAX_ENERGY_MODES; i++)
  {
      if(lowest_energy_mode[i] != 0) return i;
  }
  return (MAX_ENERGY_MODES-1);
}

/***************************************************************************//**
 * @brief
 *   Block a sleep mode.
 *
 * @details
 *   This function will be called by a peripheral to block a sleep mode that would
 *   interfere with functionality.
 *
 * @note
 *   This function uses atomic operations to ensure interrupts do not interfere.
 *
 * @param[in] EM
 *    Energy mode to block (defined in sleep_routines.h)
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
 *   Unblock a sleep mode.
 *
 * @details
 *   This function will be called by a peripheral to unblock a sleep mode that was
 *   previously blocked.
 *
 * @note
 *   This function uses atomic operations to ensure interrupts do not interfere.
 *
 * @param[in] EM
 *    Energy mode to block (defined in sleep_routines.h)
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
 *   Initialize energy mode block array
 *
 * @details
 *   Initializes all energy modes as unblocked.
 *
 * @note
 *   This function uses atomic operations to ensure interrupts do not interfere.
 *
 ******************************************************************************/
void sleep_open(void)
{
  for(int i = 0; i < MAX_ENERGY_MODES; i++) lowest_energy_mode[i] = 0;
}
