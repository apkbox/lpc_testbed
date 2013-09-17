
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
    SUBSTATE_WAIT_FOR_HEX,
    SUBSTATE_DONE
};


static enum PROTO_STATE state = STATE_START;
static enum ProtoSubstate substate = SUBSTATE_INITIAL;
static const ProtocolHandler *current_handler = NULL;

#define BUFFER_LENGTH     16
static char buffer[BUFFER_LENGTH];
static int buffer_ptr = 0;


static const ProtocolHandler *SelectModule(const char *name)
{
    const ProtocolHandler *p;

    for (p = sProtocolHandlers; p->id != 0; p++) {
        if (strcmpi(name, p->prefix) == 0)
            return p;
    }

    return NULL;
}


void PROTO_InitBuffer()
{
    buffer[0] = '\0';
    buffer_ptr = 0;
}

int PROTO_BufferAppend(char c)
{
    if (buffer_ptr >= (BUFFER_LENGTH - 1)) {
        buffer[buffer_ptr] = '\0';
        return 0;
    }
    else {
        buffer[buffer_ptr++] = c;
        return 1;
    }
}

const char *PROTO_GetBuffer()
{
    buffer[buffer_ptr] = '\0';
    return buffer;
}


void PROTO_Reset()
{
    state = STATE_START;
    current_handler = NULL;
    buffer_ptr = 0;
}


void PROTO_SetHandlers(const ProtocolHandler *protocols)
{
    sProtocolHandlers = protocols;
}


const ProtocolHandler *PROTO_GetCurrentHandler()
{
    return current_handler;
}


// TODO: Make such that protocol handlers can reuse
// common states like input number, input hex number, input list...
int PROTO_HandleInputCharacter(char c)
{
    c = toupper(c);

    if (c == ' ')
        return RESULT_NEXT_CHAR;
  
    switch (state) {
        case  STATE_START:
            current_handler = NULL;
            if (c == '@') {
                buffer_ptr = 0;
                state = STATE_WAIT_FOR_MODULE;
            }
            else if (!iscrlf(c)) {
                state = STATE_ERROR;
            }
            break;

        case STATE_WAIT_FOR_MODULE:
            if (c == ':') {
                buffer[buffer_ptr] = 0;
                current_handler = SelectModule(buffer);
                if (current_handler == NULL) {
                     state = STATE_ERROR;
                }

                state = STATE_HANDLE_PROTO_COMMAND;
            }
            else if (buffer_ptr < (BUFFER_LENGTH - 1))
                buffer[buffer_ptr++] = c;
            else
                state = STATE_ERROR;
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
                if (iscrlf(c))
                    state = STATE_ERROR;
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
    PROTO_InitBuffer();
    substate = SUBSTATE_INITIAL;
}



enum PROTO_RESULT PROTO_ParseNumber(char c)
{
    switch (substate) {
        case SUBSTATE_INITIAL:
            if (!isdigit(c))
                return RESULT_ACCEPT;

            substate = SUBSTATE_WAIT_FOR_DEC;
            if (c == '0')
                substate = SUBSTATE_OCT_OR_HEX;

            // TODO: Handle overflow
            PROTO_BufferAppend(c);
            break;

        case SUBSTATE_OCT_OR_HEX:
            if (c == 'x' || c == 'X') {
                // TODO: Handle overflow
                PROTO_BufferAppend(c);
                substate = SUBSTATE_WAIT_FOR_HEX;
                break;
            }
            /* skip to SUBSTATE_WAIT_FOR_OCT */

        case SUBSTATE_WAIT_FOR_OCT:
            if (!isdigit(c) || c >= '8')
                return RESULT_ACCEPT;

            // TODO: Handle overflow
            PROTO_BufferAppend(c);
            break;

        case SUBSTATE_WAIT_FOR_DEC:
            if (!isdigit(c)) {
                state = SUBSTATE_INITIAL;
                return RESULT_ACCEPT;
            }

            // TODO: Handle overflow
            PROTO_BufferAppend(c);
            break;

        case SUBSTATE_WAIT_FOR_HEX:
            if (!isxdigit(c)) {
                state = SUBSTATE_INITIAL;
                return RESULT_ACCEPT;
            }

            // TODO: Handle overflow
            PROTO_BufferAppend(c);
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
static ModuleHandlerFunc module = 0;
#define BUFFER_LENGTH     32
static char buffer[BUFFER_LENGTH];
static int buffer_ptr = 0;

static COMMAND sequence[MAX_SEQUENCE_LENGTH];
static int sequence_ptr = 0;

static int current_delay;
static unsigned long current_color;
static unsigned char backlite_power = 0;
*/

/*
int PROTO_GetCommand()
{
  return command;
}

COMMAND *PROTO_GetSequence()
{
  return sequence;
}

int PROTO_GetSequenceLength()
{
  return sequence_ptr;
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

        case STATE_EXPECT_PROFILE:
          if( c == 'L' || c == 'T' ) {
            state = STATE_DELAY;
            buffer_ptr = 0;
          }
          else if( c == '\n' || c == '\r' ) {
            if( sequence_ptr == 0 ) {
              command = COMMAND_PRINT_SEQUENCE;
            }
            else {
              command = COMMAND_SET_SEQUENCE;
            }
            state = STATE_START;
            return RESULT_ACCEPT;
          }
          else
            state = STATE_ERROR;
          break;

        case STATE_DELAY:
          if( isdigit( c ) ) {
            if( buffer_ptr >= BUFFER_LENGTH )
              state = STATE_ERROR;
            else
              buffer[buffer_ptr ++] = c;
          }
          else if( c == '#' ) {
            buffer[buffer_ptr] = '\0';
            current_delay = atoi( buffer );
            state = STATE_COLOR;
            buffer_ptr = 0;
          }
          break;

        case STATE_COLOR:
          if( isxdigit( c ) ) {
            buffer[buffer_ptr ++] = c;
            if( buffer_ptr >= 6 ) {
              buffer[buffer_ptr] = '\0';
              current_color = strtoul( buffer, 0, 16 );

              if( sequence_ptr >= MAX_SEQUENCE_LENGTH ) {
                state = STATE_ERROR;
              }
              else {
                sequence[sequence_ptr].interval = current_delay;
                sequence[sequence_ptr].color.red = ( current_color >> 16 ) & 0xFF;
                sequence[sequence_ptr].color.green = ( current_color >> 8 ) & 0xFF;
                sequence[sequence_ptr].color.blue = current_color & 0xFF;
                sequence_ptr ++;
                state = STATE_EXPECT_PROFILE;
              }
            }
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

