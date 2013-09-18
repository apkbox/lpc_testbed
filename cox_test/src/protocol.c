
#include <string.h>
#include <stdlib.h>

#include "string_util.h"
#include "protocol.h"

static const ProtocolHandler *sProtocolHandlers = NULL;


enum PROTO_STATE {
    STATE_START,
    STATE_WAIT_FOR_MODULE,
    STATE_HANDLE_PROTO_COMMAND,
    STATE_WAIT_FOR_EOL,
    STATE_ERROR
};


enum ProtoSubstate {
    SUBSTATE_INITIAL,
    SUBSTATE_OCT_OR_HEX,
    SUBSTATE_WAIT_FOR_OCT,
    SUBSTATE_WAIT_FOR_DEC,
    SUBSTATE_WAIT_FOR_HEX
};


static enum PROTO_STATE state = STATE_START;
static enum ProtoSubstate substate = SUBSTATE_INITIAL;
static const ProtocolHandler *current_handler = NULL;

#define PROTO_BUFFER_LENGTH     16
static char proto_buffer[PROTO_BUFFER_LENGTH];
static int proto_buffer_ptr = 0;


static const ProtocolHandler *SelectModule(const char *name)
{
    const ProtocolHandler *p;

    for (p = sProtocolHandlers; p->id != 0; p++) {
        if (strcmpi(name, p->prefix) == 0)
            return p;
    }

    return NULL;
}


void PROTOBUF_Init()
{
    proto_buffer[0] = '\0';
    proto_buffer_ptr = 0;
}

int PROTOBUF_Append(char c)
{
    if (proto_buffer_ptr >= (PROTO_BUFFER_LENGTH - 1)) {
        proto_buffer[proto_buffer_ptr] = '\0';
        return 0;
    }
    else {
        proto_buffer[proto_buffer_ptr++] = c;
        return 1;
    }
}

const char *PROTOBUF_GetBuffer()
{
    proto_buffer[proto_buffer_ptr] = '\0';
    return proto_buffer;
}

int PROTOBUF_GetLength()
{
    return proto_buffer_ptr;
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

    if (c == ' ')
        return RESULT_NEXT_CHAR;
  
    switch (state) {
        case  STATE_START:
            current_handler = NULL;
            if (c == '@') {
                PROTOBUF_Init();
                state = STATE_WAIT_FOR_MODULE;
            }
            else if (!iscrlf(c)) {
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


void PROTO_ResetSubparser()
{
    PROTOBUF_Init();
    substate = SUBSTATE_INITIAL;
}


enum PROTO_RESULT PROTO_ParseNumber(char c)
{
    switch (substate) {
        case SUBSTATE_INITIAL:
            if (!isdigit(c))
                return RESULT_ERROR;

            substate = SUBSTATE_WAIT_FOR_DEC;

            if (c == '0')
                substate = SUBSTATE_OCT_OR_HEX;

            if (!PROTOBUF_Append(c))
                return RESULT_ERROR;
            break;

        case SUBSTATE_OCT_OR_HEX:
            if (c == 'X') {
                if (!PROTOBUF_Append(c))
                    return RESULT_ERROR;

                substate = SUBSTATE_WAIT_FOR_HEX;
                break;
            }
            /* skip to SUBSTATE_WAIT_FOR_OCT */

        case SUBSTATE_WAIT_FOR_OCT:
            if (!isdigit(c))
                return RESULT_ACCEPT;

             if (c >= '8')
                return RESULT_ERROR;

             if (!PROTOBUF_Append(c))
                 return RESULT_ERROR;
            break;

        case SUBSTATE_WAIT_FOR_DEC:
            if (!isdigit(c))
                return RESULT_ACCEPT;

            if (!PROTOBUF_Append(c))
                return RESULT_ERROR;
            break;

        case SUBSTATE_WAIT_FOR_HEX:
            if (!isxdigit(c)) {
                // Check that we have '0x'
                if (PROTOBUF_GetLength() > 2)
                    return RESULT_ACCEPT;
                else
                    return RESULT_ERROR;
            }

            // TODO: Handle overflow
            if (!PROTOBUF_Append(c))
                return RESULT_ERROR;
            break;
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
static int command = COMMAND_NONE;

static unsigned char backlite_power = 0;
*/

/*
int PROTO_GetCommand()
{
  return command;
}

unsigned char PROTO_GetBacklitePower()
{
    return backlite_power;
}

*/


/*
    case STATE_PLUS:
      if( c == '\n' || c == '\r' ) {
        command = COMMAND_PLUS;
        state = STATE_START;
        return RESULT_ACCEPT;
      }
      else if( c == 'R' ) {
        command = COMMAND_RESET;
        state = STATE_WAIT_FOR_EOL;
      }
      else if (c == 'B') {
          command = COMMAND_BACKLITE;
          state = STATE_BL_INTENSITY;
          buffer_ptr = 0;
      }
      else if( c == 'V' ) {
        command = COMMAND_VERSION;
        state = STATE_WAIT_FOR_EOL;
      }
      else if( c == 'S' ) {
        sequence_ptr = 0;
        state = STATE_EXPECT_PROFILE;
      }
      else if( c == 'C' ) {
        command = COMMAND_CAPABILITIES;
        state = STATE_WAIT_FOR_EOL;
      }
      else {
        state = STATE_ERROR;
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

