#include "ks0066.h"

#include "system_LPC11xx.h"


static void delay_ns(int ns)
{
    volatile unsigned int cycles = ns / (1000000000 / SystemCoreClock);
    while (cycles--);
}


void KS0066_PowerUpDelay()
{
    delay_ns(30000000);
}


void KS0066_Write(uint8_t data, uint8_t write_data)
{
    GPIO_SetDir(LCD_DATA_PORT, LCD_DATA_BUS, 1);
    GPIO_MaskedWrite(LCD_DATA_PORT, LCD_DATA_BUS, data);

    GPIO_MaskedWrite(LCD_RS_PORT, LCD_RS_PIN, write_data ? LCD_RS_PIN : 0);
    GPIO_ResetBits(LCD_RW_PORT, LCD_RW_PIN);
    delay_ns(40);
    GPIO_SetBits(LCD_E_PORT, LCD_E_PIN);
    delay_ns(195);
    GPIO_ResetBits(LCD_E_PORT, LCD_E_PIN);
}


uint8_t KS0066_Read(uint8_t read_ram)
{
    uint8_t value;

    GPIO_SetDir(LCD_DATA_PORT, LCD_DATA_BUS, 0);

    GPIO_MaskedWrite(LCD_RS_PORT, LCD_RS_PIN, read_ram ? LCD_RS_PIN : 0);
    GPIO_SetBits(LCD_RW_PORT, LCD_RW_PIN);
    delay_ns(40);
    GPIO_SetBits(LCD_E_PORT, LCD_E_PIN);
    delay_ns(120);
    value = GPIO_ReadInput(LCD_DATA_PORT);
    GPIO_ResetBits(LCD_E_PORT, LCD_E_PIN);
    GPIO_ResetBits(LCD_RW_PORT, LCD_RW_PIN);
    GPIO_SetDir(LCD_DATA_PORT, LCD_DATA_BUS, 1);

    return value;
}


uint8_t KS0066_IsBusy()
{
    return KS0066_Read(0) & 0x80 ? 1 : 0;
}


void KS0066_WaitForIdle()
{
    while (KS0066_IsBusy());
}


void KS0066_FunctionSet()
{
    KS0066_Write(KS0066_FUNCTION_SET_CMD |
            KS0066_8_BIT_INTERFACE |
            KS0066_2_LINE_MODE |
            KS0066_5BY11_FONT, 0);
}


void KS0066_DisplayOnOffControl(uint8_t dispctl)
{
    KS0066_Write(KS0066_DISPLAY_ON_OFF_CMD | (dispctl & KS0066_DISPLAY_ON_OFF_MASK), 0);
}


void KS0066_ClearDisplay()
{
    KS0066_Write(KS0066_CLEAR_DISPLAY_CMD, 0);
}


void KS0066_SetDDRAMAddress(uint8_t addr)
{
    KS0066_Write(KS0066_SET_DDRAM_ADDRESS_CMD | (addr & KS0066_SET_DDRAM_ADDRESS_MASK), 0);
}


void KS0066_WriteDataToRam(uint8_t data)
{
    KS0066_Write(data, 1);
}


void KS0066_WriteString(char *str, uint8_t flags)
{
    char *ptr = str;
    uint8_t col = 0;
    uint8_t row = 0;

    KS0066_SetDDRAMAddress(0);
    KS0066_WaitForIdle();

    while (*ptr) {
        KS0066_Write(*ptr++, 1);
        KS0066_WaitForIdle();
#if LCD_LINES > 1
        if (flags & KS0066_WRAP_FLAG) {
            if (++col >= LCD_COLUMNS) {
                if (++row >= LCD_LINES)
                    break;

                col = 0;
                KS0066_SetDDRAMAddress(row * KS0066_MAX_COLUMNS);
                KS0066_WaitForIdle();
            }
        }
        else {
#endif
            if (col >= (LCD_COLUMNS - 1))
                break;
#if LCD_LINES > 1
        }
#endif
    }
}

