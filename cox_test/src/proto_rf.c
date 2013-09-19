#include "proto_rf.h"

#include <stdlib.h>

#include "protocol.h"
#include "protobuf.h"
#include "string_util.h"


typedef enum RFID_PROTO_STATES_ {
    STATE_INITIAL,
    STATE_EXPECT_TARGET,
    STATE_EXPECT_ADDRESS,
    STATE_EXPECT_MODE,
    STATE_EXPECT_LENGTH,
    STATE_EXPECT_DATA
} RFID_PROTO_STATES;


static RFID_PROTO_STATES state;
static RFID_PROTO_ACTION action;
static RFID_PROTO_TARGET target;
static RFID_PROTO_MODE mode;
static int address;
static int length;

#define DATA_BUFFER_LEN    64
static uint8_t data[DATA_BUFFER_LEN];


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


uint8_t *PROTO_RF_GetData()
{
    return data;
}


enum PROTO_RESULT PROTO_RF_ProtocolHandler(char c)
{
    enum PROTO_RESULT result;

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
            PROTO_ResetSubparser();
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
            if (isxdigit(c)) {
                if (!PROTOBUF_Append(c)) {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
            }
            else if (c == ':') {
                if (PROTOBUF_GetLength() == 0) {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
                else {
                    address = strtoul(PROTOBUF_GetBuffer(), NULL, 0);
                    if (action == PROTO_RF_ACTION_READ) {
                        state = STATE_EXPECT_LENGTH;
                    }
                    else {
                        state = STATE_EXPECT_DATA;
                        length = 0;
                    }

                    PROTO_ResetSubparser();
                }
            }
            else if (iscrlf(c) && PROTOBUF_GetLength() > 0 && action == PROTO_RF_ACTION_READ) {
                address = strtoul(PROTOBUF_GetBuffer(), NULL, 0);
                length = 1;
                state = STATE_INITIAL;
                return RESULT_ACCEPT;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_LENGTH:
            result = PROTO_ParseNumber(c);
            if (result == RESULT_ACCEPT) {
                length = strtoul(PROTOBUF_GetBuffer(), NULL, 0);
                if (iscrlf(c)) {
                    state = STATE_INITIAL;
                    return RESULT_ACCEPT;
                }
                else {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
            }
            else if (result == RESULT_ERROR) {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_DATA:
            result = PROTO_ParseNumber(c);
            if (result == RESULT_ACCEPT) {
                if (length >= DATA_BUFFER_LEN) {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }

                data[length++] = strtoul(PROTOBUF_GetBuffer(), NULL, 0);
                PROTO_ResetSubparser();

                if (iscrlf(c)) {
                    state = STATE_INITIAL;
                    return RESULT_ACCEPT;
                }
                else if (c != ' ' && c != ',') {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
            }
            else if (result == RESULT_ERROR) {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

    }

    return RESULT_NEXT_CHAR;
}


void PROTO_RF_ProtocolReset()
{
    state = STATE_INITIAL;
}
