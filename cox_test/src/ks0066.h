/*
 * ks0066.h
 *
 *  Created on: Sep 9, 2013
 *      Author: akozl
 */

#ifndef KS0066_H_
#define KS0066_H_

#include "ks0066_config.h"

#define KS0066_DISPCTL_DISPLAY_ON   (1<<2)
#define KS0066_DISPCTL_DISPLAY_OFF  (0<<2)

#define KS0066_DISPCTL_CURSOR_ON   (1<<1)
#define KS0066_DISPCTL_CURSOR_OFF  (0<<1)

#define KS0066_DISPCTL_BLINK_ON   (1<<0)
#define KS0066_DISPCTL_BLINK_OFF  (0<<0)

#define KS0066_CLEAR_DISPLAY_CMD   0x01

#define KS0066_DISPLAY_ON_OFF_CMD   0x08
#define KS0066_DISPLAY_ON_OFF_MASK  0x07


#define KS0066_SET_DDRAM_ADDRESS_CMD    0x80
#define KS0066_SET_DDRAM_ADDRESS_MASK   0x7F

#define KS0066_FUNCTION_SET_CMD         0x20
#define KS0066_8_BIT_INTERFACE          (1<<4)
#define KS0066_4_BIT_INTERFACE          (0<<4)
#define KS0066_1_LINE_MODE              (0<<3)
#define KS0066_2_LINE_MODE              (1<<3)
#define KS0066_5BY8_FONT                (0<<2)
#define KS0066_5BY11_FONT               (1<<2)

#define KS0066_MAX_COLUMNS     0x40


#define KS0066_WRAP_FLAG   0x01

void KS0066_PowerUpDelay();

void KS0066_Write(uint8_t data, uint8_t write_data);
uint8_t KS0066_Read(uint8_t read_ram);
uint8_t KS0066_IsBusy();

void KS0066_WaitForIdle();
void KS0066_FunctionSet();
void KS0066_DisplayOnOffControl(uint8_t dispctl);
void KS0066_ClearDisplay();
void KS0066_SetDDRAMAddress(uint8_t addr);
void KS0066_WriteDataToRam(uint8_t data);
void KS0066_WriteString(char *str, uint8_t flags);


#endif /* KS0066_H_ */
