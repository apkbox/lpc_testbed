#if !defined( __SEQUENCER_H__ )
#define __SEQUENCER_H__

#include <stdint.h>

#define MAX_SEQUENCE_LENGTH     32


#define TICKS_PER_SECOND          100
#define TICKS_PER_INTERVAL        (1000 / TICKS_PER_SECOND)

#define TRANSITION_IMMEDIATE      0
#define TRANSITION_LINEAR         1
#define TRANSITION_LOG            2
#define TRANSITION_EXP            3

typedef struct RGBCOLOR_ {
  uint8_t    red;
  uint8_t    green;
  uint8_t    blue;
} RGBCOLOR;

typedef struct COMMAND_ {
  int  transition: 2;
  int  interval: 6;
  RGBCOLOR color;
} COMMAND;


#ifdef __cplusplus
extern "C" {
#endif

void SEQ_Initialize();
void SEQ_SetSequence(const COMMAND *sequence, int length );
const COMMAND *SEQ_GetSequence();
int SEQ_GetSequenceLength();
int SEQ_Tick();

int SEQ_CurrentRed();
int SEQ_CurrentGreen();
int SEQ_CurrentBlue();

#ifdef __cplusplus
}
#endif

#endif // __SEQUENCER_H__


