#include "lpc11xx_gpio.h"
#include "lpc11xx_ssp.h"

#include "cl632_config.h"


static void delay_ns(int ns)
{
    volatile unsigned int cycles = ns / (1000000000 / SystemCoreClock);
    while (cycles--);
}


void CL632_Init()
{
    SSP_CFG_Type ssp_config;

    if (CL632_SPI == LPC_SSP0)
        SSP_SSP0PinsInit(SCK0_PIO0_6, DISABLE);
    else if (CL632_SPI == LPC_SSP1)
        SSP_SSP1PinsInit(DISABLE);
    else
        return;

    ssp_config.Databit = SSP_DATABIT_8;
    ssp_config.CPOL = SSP_CPOL_HI;
    ssp_config.CPHA = SSP_CPHA_FIRST;
    ssp_config.Mode = SSP_MASTER_MODE;
    ssp_config.FrameFormat = SSP_FRAME_SPI;
    ssp_config.ClockRate = 16000;
    SSP_Init(CL632_SPI, &ssp_config);

    CL632_COFNIGURE_SSEL_PIN();

    GPIO_SetDir(CL632_SSEL_PORT,  CL632_SSEL_PIN, 1);
    GPIO_SetBits(CL632_SSEL_PORT, CL632_SSEL_PIN);
    SSP_Cmd(CL632_SPI, ENABLE);
}


inline void CL632_SpiWaitForTxEmpty()
{
    while (SSP_GetStatus(CL632_SPI, SSP_STAT_TXFIFO_EMPTY) == RESET);
}

inline void CL632_SpiWaitForTxReady()
{
    while (SSP_GetStatus(CL632_SPI, SSP_STAT_TXFIFO_NOTFULL) == RESET);
}


inline void CL632_SpiFlushRxFifo()
{
    while (SSP_GetStatus(CL632_SPI, SSP_STAT_RXFIFO_NOTEMPTY) == SET)
        SSP_ReceiveData(CL632_SPI);
}

inline void CL632_SpiWaitForRx()
{
    while (SSP_GetStatus(CL632_SPI, SSP_STAT_RXFIFO_NOTEMPTY) == RESET);
}


inline void CL632_SpiSselStart()
{
    GPIO_ResetBits(CL632_SSEL_PORT, CL632_SSEL_PIN);
}


inline void CL632_SpiSselStop()
{
    GPIO_SetBits(CL632_SSEL_PORT, CL632_SSEL_PIN);
}


void CL632_SpiWriteByte(uint8_t address, uint8_t data)
{
    CL632_SpiWaitForTxEmpty();

    CL632_SpiSselStart();
    delay_ns(100);

    SSP_SendData(CL632_SPI, (address & 0x3F) << 1);
    SSP_SendData(CL632_SPI, data);
    CL632_SpiWaitForTxEmpty();

    delay_ns(100);
    CL632_SpiSselStop();
}


uint8_t CL632_SpiReadByte(uint8_t address)
{
    CL632_SpiWaitForTxEmpty();
    CL632_SpiFlushRxFifo();

    CL632_SpiSselStart();

    delay_ns(100);
    SSP_SendData(CL632_SPI, 0x80 | ((address & 0x3F) << 1));
    SSP_SendData(CL632_SPI, 0);
    CL632_SpiWaitForTxEmpty();

    delay_ns(100);
    CL632_SpiSselStop();

    SSP_ReceiveData(CL632_SPI);
    return SSP_ReceiveData(CL632_SPI);
}


void CL632_SpiWrite(uint8_t address, uint8_t same_address, const uint8_t *data, uint16_t len)
{
    if (len == 0)
        return;

    CL632_SpiWaitForTxEmpty();

    CL632_SpiSselStart();
    delay_ns(100);

    SSP_SendData(CL632_SPI, (address & 0x3F) << 1);
    while (len-- > 0) {
        SSP_SendData(CL632_SPI, *data++);
        if (!len)
            break;

        if (!same_address) {
            CL632_SpiSselStop();
            delay_ns(100);
            CL632_SpiSselStart();
            address++;
            SSP_SendData(CL632_SPI, (address & 0x3F) << 1);
        }

        CL632_SpiWaitForTxReady();
    }

    CL632_SpiWaitForTxEmpty();

    delay_ns(100);
    CL632_SpiSselStop();
}


void CL632_SpiRead(uint8_t address, uint8_t same_address, uint8_t *data, uint16_t len)
{
    if (len == 0)
        return;

    CL632_SpiWaitForTxEmpty();
    CL632_SpiFlushRxFifo();

    CL632_SpiSselStart();
    delay_ns(100);

    SSP_SendData(CL632_SPI, 0x80 | ((address & 0x3F) << 1));
    CL632_SpiWaitForRx();
    SSP_ReceiveData(CL632_SPI);
    while (len-- > 1) {
        if (!same_address)
            address++;

        SSP_SendData(CL632_SPI, (address & 0x3F) << 1);
        CL632_SpiWaitForRx();
        *data++ = SSP_ReceiveData(CL632_SPI);
    }

    SSP_SendData(CL632_SPI, 0);
    CL632_SpiWaitForRx();
    *data++ = SSP_ReceiveData(CL632_SPI);

    delay_ns(100);
    CL632_SpiSselStop();
}


int CL632_ReadFIFOLen()
{
    return CL632_SpiReadByte(0x04);
}


void CL632_ReadE2(uint8_t address, uint8_t *data, uint8_t len)
{
    const uint8_t readE2 = 0x03;
    uint8_t params[3];

    params[0] = address & 0xff;
    params[1] = (address >> 8) & 0xff;
    params[2] = len;

    // Write to FIFO
    CL632_SpiWrite(0x02, 1, params, 3);

    // Command
    CL632_SpiWriteByte(0x01, readE2);

    // Read result
    CL632_SpiRead(0x02, 1, data, len);
}


void CL632_WriteE2(uint8_t address, const uint8_t *data, uint8_t len)
{
    const uint8_t writeE2 = 0x01;
    uint8_t params[2];

    params[0] = address & 0xff;
    params[1] = (address >> 8) & 0xff;

    // Write to FIFO
    CL632_SpiWrite(0x02, 1, params, 2);
    CL632_SpiWrite(0x02, 1, data, len);

    // Command
    CL632_SpiWriteByte(0x01, writeE2);
}
