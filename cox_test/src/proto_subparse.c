#include "protocol.h"

#include <stdlib.h>

#include "string_util.h"
#include "protobuf.h"


enum ProtoSubstate {
    SUBSTATE_INITIAL,
    SUBSTATE_OCT_OR_HEX,
    SUBSTATE_WAIT_FOR_OCT,
    SUBSTATE_WAIT_FOR_DEC,
    SUBSTATE_WAIT_FOR_HEX
};


static enum ProtoSubstate substate = SUBSTATE_INITIAL;


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

            if (!PROTOBUF_Append(c))
                return RESULT_ERROR;
            break;
    }

    return RESULT_NEXT_CHAR;
}
