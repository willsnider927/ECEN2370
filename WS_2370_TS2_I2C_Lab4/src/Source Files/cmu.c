/**
 * @file cmu.c
 * @author Will Snider
 * @date 09/07/2021
 * @brief
 * Opens and configures the CMU clock tree to use the correct timer and routing.
 *
 */
//***********************************************************************************
// Include files
//***********************************************************************************
#include "cmu.h"

//***********************************************************************************
// defined files
//***********************************************************************************


//***********************************************************************************
// Private variables
//***********************************************************************************


//***********************************************************************************
// Private functions
//***********************************************************************************


//***********************************************************************************
// Global functions
//***********************************************************************************

/***************************************************************************//**
 * @brief
 * Driver to open the CMU clock and route to the correct branch.
 *
 * @details
 * Disables unused CMU clocks and routes the UFLRCO Ocillator to the correct
 * low frequency clock tree.
 *
 * @note
 * Routing must still be done within individual peripheral initialization functions.
 * First driver to be called so that routing can be checked within other drivers.
 *
 ******************************************************************************/

void cmu_open(void){

    CMU_ClockEnable(cmuClock_HFPER, true);

    // By default, LFRCO is enabled, disable the LFRCO oscillator
    // Disable the LFRCO oscillator
    // What is the enumeration required for LFRCO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFRCO , false, false);

    // Disable the LFXO oscillator
    // What is the enumeration required for LFXO?
    // It can be found in the online HAL documentation
    CMU_OscillatorEnable(cmuOsc_LFXO, false, false);

    // No requirement to enable the ULFRCO oscillator.  It is always enabled in EM0-4H1

    // Route LF clock to the LF clock tree
    // What is the enumeration required to placed the ULFRCO onto the proper clock branch?
    // It can be found in the online HAL documentation
    CMU_ClockSelectSet(cmuClock_LFA , cmuSelect_ULFRCO);    // routing ULFRCO to proper Low Freq clock tree

    // What is the proper enumeration to enable the clock tree onto the LE clock branches?
    // It can be found in the Assignment 2 documentation
    CMU_ClockEnable(cmuClock_CORELE, true);

}

