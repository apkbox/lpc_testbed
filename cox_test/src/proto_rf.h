#ifndef PROTO_RF_H_
#define PROTO_RF_H_

#include <stdint.h>
#include "protocol.h"

#define RF_PROTOCOL_PREFIX   "RF"

typedef enum RFID_PROTO_ACTION_ {
    PROTO_RF_ACTION_READ = 'R',
    PROTO_RF_ACTION_WRITE = 'W'
} RFID_PROTO_ACTION;

typedef enum RFID_PROTO_TARGET_ {
    PROTO_RF_TARGET_PORT = 'P',
    PROTO_RF_TARGET_MEMORY = 'M'
} RFID_PROTO_TARGET;

typedef enum RFID_PROTO_MODE_ {
    PROTO_RF_MODE_SAME_ADDRESS,
    PROTO_RF_MODE_INCREMENT_ADDRESS
} RFID_PROTO_MODE;


enum PROTO_RESULT PROTO_RF_ProtocolHandler(char c);
void PROTO_RF_ProtocolReset();
RFID_PROTO_ACTION PROTO_RF_GetAction();
RFID_PROTO_TARGET PROTO_RF_GetTarget();
RFID_PROTO_MODE PROTO_RF_GetMode();
int PROTO_RF_GetAddress();
int PROTO_RF_GetLength();
uint8_t *PROTO_RF_GetData();


#endif /* PROTO_RF_H_ */
