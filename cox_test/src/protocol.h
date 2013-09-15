#if !defined( __PROTOCOL_H__ )
#define __PROTOCOL_H__


typedef enum PROTO_RESULT_ {
    RESULT_NEXT_CHAR,
    RESULT_ACCEPT,
    RESULT_ERROR
} PROTO_RESULT;


/*
typedef enum COMMANDS_ {
    COMMAND_NONE,
    COMMAND_RESET,
    COMMAND_PLUS,
    COMMAND_MINUS,
    COMMAND_BACKLITE,
    COMMAND_VERSION,
    COMMAND_CAPABILITIES,
    COMMAND_PRINT_SEQUENCE,
    COMMAND_SET_SEQUENCE,
    COMMAND_READ_SPI,
    COMMAND_READ_PORT,
    COMMAND_WRITE_PORT,
    COMMAND_READ_EEPROM,
    COMMAND_WRITE_EEPROM
} COMMANDS;
*/


typedef PROTO_RESULT (*ProtocolHandlerFunc)(char c);


typedef struct ProtocolHandler_ {
    int id;
    char *prefix;
    ProtocolHandlerFunc handler;
} ProtocolHandler;


void PROTO_Reset();
void PROTO_SetHandlers(const ProtocolHandler *protocols);
ProtocolHandler *PROTO_GetCurrentHandler();
int PROTO_HandleInputCharacter(char c);


int PROTO_GetCommand();
//COMMAND *PROTO_GetSequence();
//int PROTO_GetSequenceLength();
//void PROTO_Reset();
//unsigned char PROTO_GetBacklitePower();


#endif // __PROTOCOL_H__ 

