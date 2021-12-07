//***********************************************************************************
// Include files
//***********************************************************************************
#ifndef SI1133_HG
#define SI1133_HG

/* System include statements */

/* Silicon Labs include statements */
#include "em_assert.h"
#include "em_i2c.h"
#include "em_gpio.h"

/* The developer's include statements */
#include "brd_config.h"
#include "HW_delay.h"
#include "i2c.h"

//***********************************************************************************
// defined files
//***********************************************************************************
#define I2Cx                    I2C1
#define Si1133_STARTUP_TIME     25

#define LOW4BIT_MASK            0x0F

#define NULL_CB                 0x00

#define Si1133_ADDRESS          0x55
#define Si1133_PART_ID          0x33
#define Si1133_ID_REG           0x00
#define Si1133_RESPONSE0_REG    0x11
#define Si1133_INPUT0_REG       0x0A
#define Si1133_COMMAND_REG      0x0B
#define Si1133_HOSTOUT1         0x14

#define Si1133_WHITEPHOTODIODE  0x0B
#define Si1133_PARAM_WRITE      0x80
#define Si1133_CONFIG0_PARAM    0x02
#define Si1133_CHANNEL0_SEL     0x01
#define Si1133_CHAN_LIST        0x01
#define Si1133_FORCE_COMMAND    0x11

//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void Si1133_i2c_open(void);
void Si1133_read(uint32_t reg, int size, int callback);
void Si1133_force_return(uint32_t callback);
void Si1133_force(void);
uint32_t Si1133_read_result(void);


#endif
