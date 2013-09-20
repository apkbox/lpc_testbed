#include "protocol.h"

#include <string.h>
#include <stdlib.h>

#include "string_util.h"
#include "protobuf.h"


static const ProtocolHandler *sProtocolHandlers = NULL;

enum PROTO_STATE {
    STATE_START,
    STATE_WAIT_FOR_T,
    STATE_WAIT_FOR_MODULE,
    STATE_HANDLE_PROTO_COMMAND,
    STATE_WAIT_FOR_EOL,
    STATE_ERROR
};


static enum PROTO_STATE state = STATE_START;
static const ProtocolHandler *current_handler = NULL;

static const ProtocolHandler g_default_handler = {
        PROTO_ID_DEFAULT,
        NULL,
        NULL,
        NULL
};


static const ProtocolHandler *SelectModule(const char *name)
{
    const ProtocolHandler *p;

    for (p = sProtocolHandlers; p->id != 0; p++) {
        if (strcmpi(name, p->prefix) == 0)
            return p;
    }

    return NULL;
}


void PROTO_Reset()
{
    state = STATE_START;
    current_handler = NULL;
    PROTO_ResetSubparser();
}


void PROTO_SetHandlers(const ProtocolHandler *protocols)
{
    sProtocolHandlers = protocols;
}


const ProtocolHandler *PROTO_GetCurrentHandler()
{
    return current_handler;
}


int PROTO_HandleInputCharacter(char c)
{
    c = toupper(c);

    // if (c == ' ')
    //    return RESULT_NEXT_CHAR;
  
    switch (state) {
        case  STATE_START:
            current_handler = NULL;
            if (c == '@') {
                PROTOBUF_Init();
                state = STATE_WAIT_FOR_MODULE;
            }
            else if (c == 'A') {
                state = STATE_WAIT_FOR_T;
            }
            else if (!iscrlf(c) && c != ' ') {
                state = STATE_ERROR;
            }
            break;

        case STATE_WAIT_FOR_T:
            if (c == 'T') {
                current_handler = &g_default_handler;
                state = STATE_WAIT_FOR_EOL;
            }
            else {
                state = STATE_ERROR;
            }
            break;

        case STATE_WAIT_FOR_MODULE:
            if (iscrlf(c)) {
                state = STATE_ERROR;
            }
            else if (c == ':') {
                current_handler = SelectModule(PROTOBUF_GetBuffer());
                if (current_handler == NULL) {
                     state = STATE_ERROR;
                }

                current_handler->reset();
                state = STATE_HANDLE_PROTO_COMMAND;
            }
            else if (!PROTOBUF_Append(c)) {
                state = STATE_ERROR;
            }
            break;

        case STATE_HANDLE_PROTO_COMMAND:
            if (current_handler == NULL) {
                state = STATE_ERROR;
            }
            else {
                switch (current_handler->handler(c)) {
                    case RESULT_NEXT_CHAR:
                        break;

                    case RESULT_ACCEPT:
                        state = STATE_START;
                        return RESULT_ACCEPT;

                    case RESULT_ERROR:
                        state = STATE_ERROR;
                        break;
                }

                // Module must handle CRLF as accept.
                // Multiline commands are not valid.
                if (iscrlf(c)) {
                    state = STATE_START;
                    state = STATE_ERROR;
                }
            }
            break;

        case STATE_WAIT_FOR_EOL:
            if (iscrlf(c)) {
                state = STATE_START;
                return RESULT_ACCEPT;
            }
            else {
                state = STATE_ERROR;
            }
            break;

        case STATE_ERROR:
            current_handler = NULL;
            if (iscrlf(c)) {
                state = STATE_START;
                return RESULT_ERROR;
            }
    }

    return RESULT_NEXT_CHAR;
}


/*
 *  Plus and minus will switch between states.
 *  +PREFIX  - switch to proto
 *  -        - switch to default prefixed mode
    if (c == '+')
        state = STATE_PLUS;
    else if (c == '-') {
        command = COMMAND_MINUS;
        state = STATE_WAIT_FOR_EOL;
    }
*/

/*
static unsigned char backlite_power = 0;

    case STATE_PLUS:
      if (c == 'B') {
          command = COMMAND_BACKLITE;
          state = STATE_BL_INTENSITY;
          buffer_ptr = 0;
      }
      break;

    case STATE_BL_INTENSITY:
      if (c == '+' && buffer_ptr == 0) {
          backlite_power = 0xff;
          state = STATE_WAIT_FOR_EOL;
      }
      else if (c == '-' && buffer_ptr == 0) {
          backlite_power = 0x00;
          state = STATE_WAIT_FOR_EOL;
      }
      else if (isxdigit(c)) {
        buffer[buffer_ptr ++] = c;
        if( buffer_ptr >= 2 ) {
          buffer[buffer_ptr] = '\0';
          backlite_power = strtoul(buffer, 0, 16);
          state = STATE_WAIT_FOR_EOL;
        }
      }
      else {
        state = STATE_ERROR;
      }
      break;
*/
