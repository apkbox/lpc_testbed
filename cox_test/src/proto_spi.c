#include "proto_spi.h"

#include <stdlib.h>

#include "protocol.h"
#include "protobuf.h"
#include "string_util.h"


typedef enum PROTO_SPI_STATES_ {
    STATE_INITIAL,
    STATE_EXPECT_DELIMITER,
    STATE_EXPECT_LENGTH,
    STATE_EXPECT_DATA
} PROTO_SPI_STATES;


static PROTO_SPI_STATES state;
static PROTO_SPI_ACTION action;
static int length;

#define DATA_BUFFER_LEN    64
static uint8_t data[DATA_BUFFER_LEN];


PROTO_SPI_ACTION PROTO_SPI_GetAction()
{
    return action;
}


int PROTO_SPI_GetLength()
{
    return length;
}


uint8_t *PROTO_SPI_GetData()
{
    return data;
}


enum PROTO_RESULT PROTO_SPI_ProtocolHandler(char c)
{
    enum PROTO_RESULT result;

    switch (state) {
        case STATE_INITIAL:
            if (c == 'R') {
                action = PROTO_SPI_ACTION_READ;
                state = STATE_EXPECT_DELIMITER;
            }
            else if(c == 'W') {
                action = PROTO_SPI_ACTION_WRITE;
                state = STATE_EXPECT_DELIMITER;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_DELIMITER:
            if (c == ':') {
                PROTO_ResetSubparser();
                length = 0;
                if (action == PROTO_SPI_ACTION_READ)
                    state = STATE_EXPECT_LENGTH;
                else if (action == PROTO_SPI_ACTION_WRITE)
                    state = STATE_EXPECT_DATA;
                else {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
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
                if (iscrlf(c) && length > 0) {
                    state = STATE_INITIAL;
                    return RESULT_ACCEPT;
                }
                else if (c != ' ') {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
            }
            break;

    }

    return RESULT_NEXT_CHAR;
}


void PROTO_SPI_ProtocolReset()
{
    state = STATE_INITIAL;
}
