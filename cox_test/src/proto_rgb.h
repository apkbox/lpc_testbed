#ifndef PROTO_RGB_H_
#define PROTO_RGB_H_

#include <stdint.h>
#include "protocol.h"
#include "sequencer.h"


#define RGB_PROTOCOL_PREFIX   "RGB"

enum PROTO_RGB_COMMAND {
    PROTO_RGB_COMMAND_CAPABILITIES = 'C',
    PROTO_RGB_COMMAND_PRINT_SEQUENCE = 'P',
    PROTO_RGB_COMMAND_SET_SEQUENCE = 'S'
};


enum PROTO_RESULT PROTO_RGB_ProtocolHandler(char c);
void PROTO_RGB_ProtocolReset();
enum PROTO_RGB_COMMAND PROTO_RGB_GetCommand();
COMMAND *PROTO_RGB_GetSequence();
int PROTO_RGB_GetSequenceLength();


#endif /* PROTO_RGB_H_ */
