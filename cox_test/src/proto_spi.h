#ifndef PROTO_SPI_H_
#define PROTO_SPI_H_

#include <stdint.h>
#include "protocol.h"

#define SPI_PROTOCOL_PREFIX   "SPI"

typedef enum PROTO_SPI_ACTION_ {
    PROTO_SPI_ACTION_READ = 'R',
    PROTO_SPI_ACTION_WRITE = 'W'
} PROTO_SPI_ACTION;

enum PROTO_RESULT PROTO_SPI_ProtocolHandler(char c);
void PROTO_SPI_ProtocolReset();
PROTO_SPI_ACTION PROTO_SPI_GetAction();
int PROTO_SPI_GetLength();
uint8_t *PROTO_SPI_GetData();


#endif /* PROTO_SPI_H_ */
