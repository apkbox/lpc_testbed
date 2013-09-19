#include "proto_rgb.h"

#include <stdlib.h>

#include "protocol.h"
#include "protobuf.h"
#include "string_util.h"


enum PROTO_RGB_STATES {
    STATE_INITIAL,
    STATE_EXPECT_PROFILE,
    STATE_DELAY,
    STATE_COLOR,
    STATE_DONE
};


static enum PROTO_RGB_STATES state;
static enum PROTO_RGB_COMMAND command;
static COMMAND sequence[MAX_SEQUENCE_LENGTH];
static int sequence_ptr = 0;
static int current_delay;
static unsigned long current_color;


enum PROTO_RGB_COMMAND PROTO_RGB_GetCommand()
{
    return command;
}

COMMAND *PROTO_RGB_GetSequence()
{
  return sequence;
}

int PROTO_RGB_GetSequenceLength()
{
  return sequence_ptr;
}


enum PROTO_RESULT PROTO_RGB_ProtocolHandler(char c)
{
    if (c == ' ')
        return RESULT_NEXT_CHAR;

    switch (state) {
        case STATE_INITIAL:
            if (c == 'S') {
                sequence_ptr = 0;
                state = STATE_EXPECT_PROFILE;
            }
            else if (c == 'C') {
                command = PROTO_RGB_COMMAND_CAPABILITIES;
                state = STATE_DONE;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_EXPECT_PROFILE:
            if (c == 'L' || c == 'T') {
                state = STATE_DELAY;
                PROTOBUF_Init();
            }
            else if (iscrlf(c)) {
                if (sequence_ptr == 0) {
                    command = PROTO_RGB_COMMAND_PRINT_SEQUENCE;
                }
                else {
                    // effectively set an empty sequence
                    command = PROTO_RGB_COMMAND_SET_SEQUENCE;
                }

                state = STATE_INITIAL;
                return RESULT_ACCEPT;
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_DELAY:
            if (isdigit(c)) {
                if (!PROTOBUF_Append(c)) {
                    state = STATE_INITIAL;
                    return RESULT_ERROR;
                }
            }
            else if (c == '#') {
                current_delay = atoi(PROTOBUF_GetBuffer());
                state = STATE_COLOR;
                PROTOBUF_Init();
            }
            break;

        case STATE_COLOR:
            if (isxdigit(c)) {
                PROTOBUF_Append(c);
                if (PROTOBUF_GetLength() >= 6) {
                    current_color = strtoul(PROTOBUF_GetBuffer(), 0, 16);

                    if (sequence_ptr >= MAX_SEQUENCE_LENGTH) {
                        state = STATE_INITIAL;
                        return RESULT_ERROR;
                    }
                    else {
                        sequence[sequence_ptr].interval = current_delay;
                        sequence[sequence_ptr].color.red = (current_color >> 16) & 0xFF;
                        sequence[sequence_ptr].color.green = (current_color >> 8) & 0xFF;
                        sequence[sequence_ptr].color.blue = current_color & 0xFF;
                        sequence_ptr++;
                        state = STATE_EXPECT_PROFILE;
                    }
                }
            }
            else {
                state = STATE_INITIAL;
                return RESULT_ERROR;
            }
            break;

        case STATE_DONE:
            state = STATE_INITIAL;
            if (iscrlf(c))
                return RESULT_ACCEPT;
            else
                return RESULT_ERROR;
    }

    return RESULT_NEXT_CHAR;
}


void PROTO_RGB_ProtocolReset()
{
    state = STATE_INITIAL;
}
