/*
 * ks0066_config.h
 *
 *  Created on: Sep 9, 2013
 *      Author: akozl
 */

#ifndef KS0066_CONFIG_H_
#define KS0066_CONFIG_H_

#include "lpc11xx_gpio.h"

#define LCD_DATA_SET_PINS() IOCON_SetPinFunc(IOCON_PIO2_0, PIO2_0_FUN_PIO); \
                            IOCON_SetPinFunc(IOCON_PIO2_1, PIO2_1_FUN_PIO); \
                            IOCON_SetPinFunc(IOCON_PIO2_2, PIO2_2_FUN_PIO); \
                            IOCON_SetPinFunc(IOCON_PIO2_3, PIO2_3_FUN_PIO_MOSI1); \
                            IOCON_SetPinFunc(IOCON_PIO2_4, PIO2_4_FUN_PIO); \
                            IOCON_SetPinFunc(IOCON_PIO2_5, PIO2_5_FUN_PIO); \
                            IOCON_SetPinFunc(IOCON_PIO2_6, PIO2_6_FUN_PIO); \
                            IOCON_SetPinFunc(IOCON_PIO2_7, PIO2_7_FUN_PIO)

#define LCD_DATA_PORT       PORT2
#define LCD_DATA_BUS        (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | \
                             GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7)

#define LCD_RS_SET_PIN()    IOCON_SetPinFunc(IOCON_PIO2_8, PIO2_8_FUN_PIO)
#define LCD_RS_PORT         PORT2
#define LCD_RS_PIN          GPIO_Pin_8

#define LCD_RW_SET_PIN()    IOCON_SetPinFunc(IOCON_PIO2_9, PIO2_9_FUN_PIO)
#define LCD_RW_PORT         PORT2
#define LCD_RW_PIN          GPIO_Pin_9

#define LCD_E_SET_PIN()     IOCON_SetPinFunc(IOCON_PIO2_10, PIO2_10_FUN_PIO)
#define LCD_E_PORT          PORT2
#define LCD_E_PIN           GPIO_Pin_10


#define LCD_COLUMNS         16
#define LCD_LINES           2

#endif /* KS0066_CONFIG_H_ */
