#include "proto_sys.h"

#include <stdlib.h>

#include "protocol.h"
#include "string_util.h"


enum PROTO_SYS_STATES {
    STATE_INITIAL,
    STATE_DONE
};


static enum PROTO_SYS_STATES state;
static enum PROTO_SYS_COMMAND command;


enum PROTO_SYS_COMMAND PROTO_SYS_GetCommand()
{
    return command;
}

enum PROTO_RESULT PROTO_SYS_ProtocolHandler(char c)
{
    switch (state) {
        case STATE_INITIAL:
            if (c == 'R') {
                command = PROTO_SYS_COMMAND_RESET;
                state = STATE_DONE;
            }
            else if (c == 'V') {
                command = PROTO_SYS_COMMAND_VERSION;
                state = STATE_DONE;
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


void PROTO_SYS_ProtocolReset()
{
    state = STATE_INITIAL;
}
