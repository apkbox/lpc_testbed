#ifndef PROTO_SYS_H_
#define PROTO_SYS_H_

#include <stdint.h>
#include "protocol.h"
#include "sequencer.h"


#define SYS_PROTOCOL_PREFIX   "SYS"

enum PROTO_SYS_COMMAND {
    PROTO_SYS_COMMAND_RESET = 'R',
    PROTO_SYS_COMMAND_VERSION = 'V'
};


enum PROTO_RESULT PROTO_SYS_ProtocolHandler(char c);
void PROTO_SYS_ProtocolReset();
enum PROTO_SYS_COMMAND PROTO_SYS_GetCommand();

#endif /* PROTO_SYS_H_ */
