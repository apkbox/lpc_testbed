/*
 * cl632_config.h
 *
 *  Created on: Sep 11, 2013
 *      Author: akozl
 */

#ifndef CL632_CONFIG_H_
#define CL632_CONFIG_H_

#include "lpc11xx_gpio.h"
#include "lpc11xx_ssp.h"

// Configuration
#define CL632_SPI           LPC_SSP0

// Does not matter if SSP1
#define CL632_SCK0          SCK0_PIO0_6

#define CL632_COFNIGURE_SSEL_PIN()  IOCON_SetPinFunc(IOCON_PIO0_2, PIO0_2_FUN_PIO)
#define CL632_SSEL_PORT     PORT0
#define CL632_SSEL_PIN      GPIO_Pin_2


#endif /* CL632_CONFIG_H_ */
