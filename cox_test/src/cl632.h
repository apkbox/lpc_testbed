#ifndef CL632_H_
#define CL632_H_

#include "lpc11xx_gpio.h"
#include "lpc11xx_ssp.h"

#include "cl632_config.h"

void CL632_Init();
uint8_t CL632_ReadFIFOLen();

void CL632_SpiWriteByte(uint8_t address, uint8_t data);
uint8_t CL632_SpiReadByte(uint8_t address);

void CL632_SpiWrite(uint8_t address, uint8_t same_address, const uint8_t *data, uint16_t len);
void CL632_SpiRead(uint8_t address, uint8_t same_address, uint8_t *data, uint16_t len);

void CL632_WriteE2(uint8_t address, const uint8_t *data, uint8_t len);
void CL632_ReadE2(uint8_t address, uint8_t *data, uint8_t len);

#endif // CL632_H_
