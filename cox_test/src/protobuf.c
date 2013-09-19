#include "protobuf.h"


#define PROTO_BUFFER_LENGTH     16
static char proto_buffer[PROTO_BUFFER_LENGTH];
static int proto_buffer_ptr = 0;


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
