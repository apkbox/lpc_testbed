#include "proto_rf.h"


typedef enum RFID_PROTO_STATES_ {
    STATE_INITIAL,
    STATE_EXPECT_TARGET,
    STATE_EXPECT_ADDRESS,
    STATE_EXPECT_MODE,
    STATE_EXPECT_DATA_DELIMITER,
    //STATE_EXPECT_BYTE,
    //STATE_EXPECT_BYTE_DELIMITER,
    STATE_EXPECT_LENGTH,
    STATE_EXPECT_DATA
} RFID_PROTO_STATES;


static RFID_PROTO_STATES state;
static RFID_PROTO_ACTION action;
static RFID_PROTO_TARGET target;
static RFID_PROTO_MODE mode;
static int address;
static int length;

#define BUFFER_LEN                 64
static uint8_t buffer[BUFFER_LEN];
static int buffer_index; 


RFID_PROTO_ACTION PROTO_RF_GetAction()
{
    return action;
}


RFID_PROTO_TARGET PROTO_RF_GetTarget()
{
    return target;
}


RFID_PROTO_MODE PROTO_RF_GetMode()
{
    return mode;
}


int PROTO_RF_GetAddress()
{
    return address;
}


int PROTO_RF_GetLength()
{
    return length;
}


void PROTO_RF_StdTokenCallback(const char *buffer)
{
}


PROTO_RESULT PROTO_RF_ProtocolHandler(char c)
{
    switch (state) {
        case STATE_INITIAL:
            if (c == 'R') {
                action = PROTO_RF_ACTION_READ;
                state = STATE_EXPECT_TARGET;
            }
            else if(c == 'W') {
                action = PROTO_RF_ACTION_WRITE;
                state = STATE_EXPECT_TARGET;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_TARGET:
            if (c == 'M') {
                target = PROTO_RF_TARGET_MEMORY;
                state = STATE_EXPECT_MODE;
            }
            else if (c == 'P') {
                target = PROTO_RF_TARGET_PORT;
                state = STATE_EXPECT_MODE;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_MODE:
            buffer_index = 0;
            if (c == '+') {
                mode = PROTO_RF_MODE_INCREMENT_ADDRESS;
                state = STATE_EXPECT_ADDRESS;
            }
            else if (c == '=') {
                mode = PROTO_RF_MODE_SAME_ADDRESS;
                state = STATE_EXPECT_ADDRESS;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_ADDRESS:
            if (iscrlf(c) && buffer_index == 0 && action == PROTO_RF_ACTION_READ) {
                length = 1;
                state = STATE_INITIAL;
                return RESULT_ACCEPT;
            }
            else if (c == ':') {
                if (buffer_index == 0) {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
                else {
                    buffer[buffer_index] = '\0';
                    address = strtoul(buffer, 0, 16);
                    if (&& action == PROTO_RF_ACTION_READ)
                        state = STATE_EXPECT_LENGTH;
                    else
                        state = STATE_EXPECT_DATA;
                }
            }
            else if (isxdigit(c)) {
                if (buffer_index >= (BUFFER_LEN - 1)) {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
                else {
                    buffer[buffer_index++] = c;
                }
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_LENGTH:
            if (iscrlf(c)) {
                state = STATE_INITIAL;
                return RESULT_ACCEPT;
            }
            else {
            }
            break;

        case STATE_EXPECT_DATA:
            state = STATE_INITIAL;
            return RESULT_ACCEPT;
    }

    return RESULT_NEXT_CHAR;
}