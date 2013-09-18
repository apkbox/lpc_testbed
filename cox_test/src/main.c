/*
===============================================================================
 Name        : main.c
 Author      : 
 Version     :
 Copyright   : Copyright (C) 
 Description : main definition
===============================================================================
*/

#include <string.h>

#include "LPC11xx.h"

#include "lpc11xx_iocon.h"
#include "lpc11xx_syscon.h"
#include "lpc11xx_gpio.h"
#include "lpc11xx_uart.h"
#include "lpc11xx_tmr.h"
#include "lpc11xx_ssp.h"

#include "small_printf.h"
#include "sequencer.h"

#include "CoOS.h"

#include "protocol.h"
#include "proto_rf.h"
#include "proto_rgb.h"

#include "ks0066.h"
#include "cl632.h"


#define RED_SET_PIN()     IOCON_SetPinFunc(IOCON_PIO1_9, PIO1_9_FUN_PIO)
#define GREEN_SET_PIN()   IOCON_SetPinFunc(IOCON_PIO1_10, PIO1_10_FUN_PIO)
#define BLUE_SET_PIN()    IOCON_SetPinFunc(IOCON_PIO1_11, PIO1_11_FUN_PIO)

#define RED_PORT          PORT1
#define GREEN_PORT        PORT1
#define BLUE_PORT         PORT1

#define RED_PIN           GPIO_Pin_9
#define GREEN_PIN         GPIO_Pin_10
#define BLUE_PIN          GPIO_Pin_11

#define TIMER_ACTIVITY_SET_PIN()   IOCON_SetPinFunc(IOCON_PIO1_4, PIO1_4_FUN_PIO)
#define TIMER_ACTIVITY_PORT        PORT1
#define TIMER_ACTIVITY_PIN         GPIO_Pin_4

#define ACTIVITY_SET_PIN()     IOCON_SetPinFunc(IOCON_PIO0_7, PIO0_7_FUN_PIO)
#define ACTIVITY_PORT          PORT0
#define ACTIVITY_PIN           GPIO_Pin_7

#define LCD_BACKLITE_SET_PIN()   IOCON_SetPinFunc(IOCON_PIO2_11, PIO2_11_FUN_PIO)
#define LCD_BACKLITE_PORT        PORT2
#define LCD_BACKLITE_PIN         GPIO_Pin_11


void UART_PrintString(const char *str)
{
    while (*str)
        UART_Send(LPC_UART, (uint8_t *)str++,  1, BLOCKING);
}

int UART_WriteChar(char c)
{
    UART_Send(LPC_UART, (uint8_t *)&c,  1, BLOCKING);
    return c;
}

int UART_ReadChar()
{
    return UART_ReceiveData(LPC_UART);
}


#define ARRAYSIZE(array)      (sizeof(array)/sizeof(array[0]))
#define GetStackTop(stack)    (&stack[ARRAYSIZE(stack)-1])
#define GetStackSize(stack)   ARRAYSIZE(stack)

static uint8_t backlite_power = 0;

enum {
    PROTO_ID_RFID = 1,
    PROTO_ID_RGB = 2
};


static const ProtocolHandler g_protocol_handlers[] = {
     { PROTO_ID_RFID, RF_PROTOCOL_PREFIX, PROTO_RF_ProtocolHandler, PROTO_RF_ProtocolReset },
     { PROTO_ID_RGB, RGB_PROTOCOL_PREFIX, PROTO_RGB_ProtocolHandler, PROTO_RGB_ProtocolReset },
     { 0, NULL, NULL }
};


void HandleInputChar(char c)
{
    int i;
    int result;

    result =  PROTO_HandleInputCharacter(c);
    if (result == RESULT_ERROR) {
        UART_PrintString("+ER\r\n");
    }
    else if( result == RESULT_ACCEPT ) {
        const ProtocolHandler *handler = PROTO_GetCurrentHandler();
        if (handler) {
            switch (handler->id) {
                case PROTO_ID_RFID:
                    if (PROTO_RF_GetAction() == PROTO_RF_ACTION_READ) {
                        int len = PROTO_RF_GetLength();
                        func_printf_nofloat(UART_WriteChar, "%02X: ", len);
                        for (i = 0; i < len; i++) {
                            int addr = PROTO_RF_GetAddress();
                            uint8_t b;

                            if (PROTO_RF_GetMode() == PROTO_RF_MODE_INCREMENT_ADDRESS)
                                addr += i;

                            if (PROTO_RF_GetTarget() == PROTO_RF_TARGET_PORT)
                                b = CL632_SpiReadByte(addr);
                            else if (PROTO_RF_GetTarget() == PROTO_RF_TARGET_MEMORY)
                                CL632_ReadE2(addr, &b, 1);

                            func_printf_nofloat(UART_WriteChar, " %02X", b);
                        }
                    }
                    else if (PROTO_RF_GetAction() == PROTO_RF_ACTION_WRITE) {
                        int addr = PROTO_RF_GetAddress();
                        uint8_t *data = PROTO_RF_GetData();
                        int len = PROTO_RF_GetLength();
                        int same_address = PROTO_RF_GetMode() == PROTO_RF_MODE_SAME_ADDRESS;

                        if (PROTO_RF_GetTarget() == PROTO_RF_TARGET_PORT) {
                            CL632_SpiWrite(addr, same_address, data, len);
                        }
                        else if (PROTO_RF_GetTarget() == PROTO_RF_TARGET_MEMORY) {
                            CL632_WriteE2(addr, data, len);
                        }
                    }

                    UART_PrintString("\r\n");
                    break;

                case PROTO_ID_RGB:
                    switch (PROTO_RGB_GetCommand()) {
                        case PROTO_RGB_COMMAND_CAPABILITIES:
                            UART_PrintString("+CAP: RGB,TRAN");
                            break;

                        case PROTO_RGB_COMMAND_PRINT_SEQUENCE:
                            UART_PrintString("+S");
                            for (i = 0; i < SEQ_GetSequenceLength(); i++) {
                                const COMMAND *cmd = &(SEQ_GetSequence()[i]);
                                func_printf_nofloat(UART_WriteChar, " L%d#%02X%02X%02X",
                                        cmd->interval, cmd->color.red, cmd->color.green, cmd->color.blue);
                            }
                            break;

                        case PROTO_RGB_COMMAND_SET_SEQUENCE:
                            SEQ_SetSequence(PROTO_RGB_GetSequence(), PROTO_RGB_GetSequenceLength());
                            UART_PrintString("+OK");
                            break;


                    }

                    UART_PrintString("\r\n");
                    break;
            }
        }
        /*
        switch( PROTO_GetCommand() ) {
            case COMMAND_PLUS:
                UART_PrintString( "+OK\r\n" );
                break;

            case COMMAND_MINUS:
                UART_PrintString( "+ER\r\n" );
                break;

            case COMMAND_RESET:
                UART_PrintString( "+RST\r\n" );
                while(UART_CheckBusy(LPC_UART) != RESET);
                NVIC_SystemReset();
                break;

            case COMMAND_BACKLITE:
                UART_PrintString( "+BL\r\n" );
                backlite_power = PROTO_GetBacklitePower();
                break;

            case COMMAND_VERSION:
                UART_PrintString( "+V:1\r\n" );
                break;
            }
        }
        */
    }
}


#define UART_TASK_PRIORITY   10
OS_STK uartTaskStack[512];


void uartTask(void *data)
{
    while (1) {
        uint32_t bytes_received;
        uint8_t c;

        bytes_received = UART_Receive(LPC_UART, &c, 1, NONE_BLOCKING);
        if (bytes_received > 0) {
            HandleInputChar(c);
            //UART_Send(LPC_UART, &c, 1, BLOCKING);
        }
    }
}


#define ACTIVITY_TASK_PRIORITY   10
OS_STK activityTaskStack[128];

void activityTask(void *data)
{
    while (1) {
        GPIO_SetBits(ACTIVITY_PORT, ACTIVITY_PIN);
        CoTimeDelay(0, 0, 1, 0);
        GPIO_ResetBits(ACTIVITY_PORT, ACTIVITY_PIN);
        CoTimeDelay(0, 0, 1, 0);
    }
}


void TIMER32_0_IRQHandler()
{
    static unsigned int duty_counter_r = 0;
    static unsigned int duty_counter_g = 0;
    static unsigned int duty_counter_b = 0;
    static unsigned int bl_duty_counter = 0;

    static unsigned int period_counter = 0;
    static int activity_counter = 0;

    CoEnterISR();

    TIM_ClearIntPending(LPC_TMR32B0, TIM_MR0_INT);

    // 39.0625us activity indicator (for calibration)
    GPIO_ToggleBits(TIMER_ACTIVITY_PORT, TIMER_ACTIVITY_PIN);

    if( period_counter -- > 0 ) {
        if( duty_counter_r -- == 0 )
            GPIO_ResetBits(RED_PORT, RED_PIN);
        if( duty_counter_g -- == 0 )
            GPIO_ResetBits(GREEN_PORT, GREEN_PIN);
        if( duty_counter_b -- == 0 )
            GPIO_ResetBits(BLUE_PORT, BLUE_PIN);

        if (bl_duty_counter-- == 0)
            GPIO_ResetBits(LCD_BACKLITE_PORT, LCD_BACKLITE_PIN);
    }
    else {
        if( activity_counter ++ >= TICKS_PER_SECOND ) {
            activity_counter = 0;
            GPIO_ToggleBits(TIMER_ACTIVITY_PORT, TIMER_ACTIVITY_PIN);

            //if( device_mode == AMBIENT_TEMPERATURE )
            //    PLUGIN_Temperature_Update();
        }

        SEQ_Tick();
        duty_counter_r = SEQ_CurrentRed();
        duty_counter_r == 0 ? GPIO_ResetBits(RED_PORT, RED_PIN) : GPIO_SetBits(RED_PORT, RED_PIN);

        duty_counter_g = SEQ_CurrentGreen();
        duty_counter_g == 0 ? GPIO_ResetBits(GREEN_PORT, GREEN_PIN ) : GPIO_SetBits(GREEN_PORT, GREEN_PIN );

        duty_counter_b = SEQ_CurrentBlue();
        duty_counter_b == 0 ? GPIO_ResetBits(BLUE_PORT, BLUE_PIN ) : GPIO_SetBits(BLUE_PORT, BLUE_PIN );

        bl_duty_counter = backlite_power;
        bl_duty_counter == 0 ? GPIO_ResetBits(LCD_BACKLITE_PORT, LCD_BACKLITE_PIN) : GPIO_SetBits(LCD_BACKLITE_PORT, LCD_BACKLITE_PIN);

        period_counter = 255;
    }

    CoExitISR();
}


int main()
{
    UART_CFG_Type uart_config;
    UART_FIFO_CFG_Type uart_fifo_config;
    TIM_TIMERCFG_Type timer_config;
    TIM_MATCHCFG_Type timer_match;
    OS_TID uart_task_id = 0;
    OS_TID activity_task_id = 0;
    uint32_t reset_flags = 0;

    reset_flags = LPC_SYSCON->SYSRSTSTAT;

    SEQ_Initialize();
    PROTO_Reset();
    PROTO_SetHandlers(g_protocol_handlers);

    SYSCON_AHBPeriphClockCmd(SYSCON_AHBPeriph_GPIO, ENABLE);

    // Reset pin
    IOCON_SetPinFunc(IOCON_PIO0_0, PIO0_0_FUN_RESET);

    // Status LED pin
    ACTIVITY_SET_PIN();
    GPIO_SetDir(ACTIVITY_PORT, ACTIVITY_PIN, 1);
    GPIO_ResetBits(ACTIVITY_PORT, ACTIVITY_PIN);

    // Timer activity LED pin
    TIMER_ACTIVITY_SET_PIN();
    GPIO_SetDir(TIMER_ACTIVITY_PORT, TIMER_ACTIVITY_PIN, 1);
    GPIO_ResetBits(TIMER_ACTIVITY_PORT, TIMER_ACTIVITY_PIN);

    // RGB control
    RED_SET_PIN();
    GREEN_SET_PIN();
    BLUE_SET_PIN();
    GPIO_SetDir(RED_PORT, RED_PIN, 1);
    GPIO_SetDir(GREEN_PORT, GREEN_PIN, 1);
    GPIO_SetDir(BLUE_PORT, BLUE_PIN, 1);
    GPIO_ResetBits(RED_PORT, RED_PIN);
    GPIO_ResetBits(GREEN_PORT, GREEN_PIN);
    GPIO_ResetBits(BLUE_PORT, BLUE_PIN);

    timer_config.PrescaleOption = TIM_PRESCALE_TICKVAL;
    timer_config.PrescaleValue = 1;
    TIM_Init(LPC_TMR32B0, TIM_TIMER_MODE, &timer_config);

    timer_match.MatchChannel = 0;
    timer_match.IntOnMatch = ENABLE;
    timer_match.StopOnMatch = DISABLE;
    timer_match.ResetOnMatch = ENABLE;
    timer_match.ExtMatchOutputType = 0;
    timer_match.MatchValue = SystemCoreClock / (TICKS_PER_SECOND * 256);
    TIM_ConfigMatch(LPC_TMR32B0, &timer_match);

    TIM_Cmd(LPC_TMR32B0, ENABLE);
    NVIC_EnableIRQ(TIMER_32_0_IRQn);

    // UART
    IOCON_SetPinFunc(IOCON_PIO1_6, PIO1_6_FUN_RXD);   /* UART RXD - PIO1_6 */
    IOCON_SetPinFunc(IOCON_PIO1_7, PIO1_7_FUN_TXD);   /* UART RXD - PIO1_7 */

    uart_config.Baud_rate = 115200;
    uart_config.Databits = UART_DATABIT_8;
    uart_config.Parity = UART_PARITY_NONE;
    uart_config.Stopbits = UART_STOPBIT_1;

    UART_Init(LPC_UART, &uart_config);

    uart_fifo_config.FIFO_Level = UART_FIFO_TRGLEV0;
    uart_fifo_config.FIFO_ResetRxBuf = ENABLE;
    uart_fifo_config.FIFO_ResetTxBuf = ENABLE;
    UART_FIFOConfig(LPC_UART, &uart_fifo_config);

    UART_TxCmd(LPC_UART, ENABLE);

    // SPI
    CL632_Init();
    // Select page 0 and no paging access
    CL632_SpiWriteByte(0x00, 0x00);
    CL632_SpiWriteByte(0x00, 0x00);

    // LCD
    // LCD backlite control
    LCD_BACKLITE_SET_PIN();
    GPIO_SetDir(LCD_BACKLITE_PORT, LCD_BACKLITE_PIN, 1);
    GPIO_ResetBits(LCD_BACKLITE_PORT, LCD_BACKLITE_PIN);

    // LCD Data bus
    LCD_DATA_SET_PINS();
    GPIO_SetDir(LCD_DATA_PORT, LCD_DATA_BUS, 1);
    GPIO_ResetBits(LCD_DATA_PORT, LCD_DATA_BUS);

    LCD_RS_SET_PIN();
    GPIO_SetDir(LCD_RS_PORT, LCD_RS_PIN, 1);
    GPIO_ResetBits(LCD_RS_PORT, LCD_RS_PIN);

    LCD_RW_SET_PIN();
    GPIO_SetDir(LCD_RW_PORT, LCD_RW_PIN, 1);
    GPIO_ResetBits(LCD_RW_PORT, LCD_RW_PIN);

    LCD_E_SET_PIN();
    GPIO_SetDir(LCD_E_PORT, LCD_E_PIN, 1);
    GPIO_ResetBits(LCD_E_PORT, LCD_E_PIN);

    KS0066_PowerUpDelay();
    KS0066_FunctionSet();
    KS0066_WaitForIdle();
    KS0066_DisplayOnOffControl(KS0066_DISPCTL_DISPLAY_ON);
    KS0066_WaitForIdle();
    KS0066_ClearDisplay();
    KS0066_WaitForIdle();

    CoInitOS();
    UART_PrintString("+OS:INIT\r\n");
    GPIO_SetBits(ACTIVITY_PORT, ACTIVITY_PIN);

    uart_task_id = CoCreateTask(uartTask, NULL, UART_TASK_PRIORITY,
            GetStackTop(uartTaskStack), GetStackSize(uartTaskStack));

    activity_task_id = CoCreateTask(activityTask, NULL, ACTIVITY_TASK_PRIORITY,
            GetStackTop(activityTaskStack), GetStackSize(activityTaskStack));

    if (uart_task_id == E_CREATE_FAIL || activity_task_id == E_CREATE_FAIL) {
        UART_PrintString("!OS:TASK\r\n");
    }

    if (reset_flags & 0x01)
        UART_PrintString("+RST:PU\r\n");
    else if (reset_flags & 0x02)
        UART_PrintString("+RST:RST\r\n");
    else if (reset_flags & 0x04)
        UART_PrintString("+RST:WDT\r\n" );
    else if (reset_flags & 0x08)
        UART_PrintString("+RST:BOD\r\n" );
    else if (reset_flags & 0x10)
        UART_PrintString("+RST:SOFT\r\n");
    else
        UART_PrintString("+RST\r\n");

    UART_PrintString("+VERINFO: " __DATE__ " " __TIME__ "\r\n");
    func_printf_nofloat(UART_WriteChar, "+COOS: %d\r\n", CoGetOSVersion());

    KS0066_WriteString("VER: " __DATE__ " " __TIME__, KS0066_WRAP_FLAG);

    CoStartOS();

    //while (1) {
    //}

    return 0;
}
