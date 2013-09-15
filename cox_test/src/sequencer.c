
#include <string.h>

#include "sequencer.h"

/*
Command:
  Transition
    7-6  Transition type (TODO: 0 interval == Sharp)
         00 - Sharp
         01 - Linear ramp
         10 - Logarithmic ramp
         11 - Exponential ramp
    5-0  100ms interval

  Color
    byte 1  Red
    byte 2  Green
    byte 3  Blue


*/

typedef struct RGBCOLOR16_ {
  WORD    red;
  WORD    green;
  WORD    blue;
} RGBCOLOR16;

static const COMMAND default_sequence[] = { 
  { TRANSITION_LINEAR, 0, { 0x00, 0x00, 0x00 } }
};

static COMMAND   sequence[MAX_SEQUENCE_LENGTH];
static int       sequence_length;
static int       sequence_index;

static RGBCOLOR16 current_color;
static COMMAND   *current_command;
static long      ticks_in_command;
static long      step_r;
static long      step_g;
static long      step_b;

static int       current_tick;

static void SetupTransition( RGBCOLOR16 *current, RGBCOLOR *next, int ticks )
{
  if( ticks == 0 ) {
    step_r = 0;
    step_g = 0;
    step_b = 0;
    return;
  }

  step_r = ( next->red * 256L - current->red ) / ticks;
  step_g = ( next->green * 256L - current->green ) / ticks;
  step_b = ( next->blue * 256L - current->blue ) / ticks;
}


static RGBCOLOR16 RgbColorToRgbColor16( RGBCOLOR *color )
{
	RGBCOLOR16 color16;
	color16.red = color->red * 256;
	color16.green = color->green * 256;
	color16.blue = color->blue * 256;
	return color16;
}

void SEQ_Initialize()
{
  current_color.red = 0;
  current_color.green = 0;
  current_color.blue = 0;

  SEQ_SetSequence( default_sequence, 1 );
}

static void SetCurrentCommand()
{
  current_command = &sequence[sequence_index];
  ticks_in_command = current_command->interval * TICKS_PER_INTERVAL;
  current_tick = 0;
  SetupTransition( &current_color, &current_command->color, ticks_in_command );
}

void SEQ_SetSequence(const COMMAND *seq, int length )
{
  memcpy(sequence, seq, sizeof( COMMAND ) * length);
  sequence_length = length;
  sequence_index = 0;

  SetCurrentCommand();
}


const COMMAND *SEQ_GetSequence()
{
    return sequence;
}


int SEQ_GetSequenceLength()
{
    return sequence_length;
}


// Executes on every interval.
int SEQ_Tick()
{
  if( current_command == 0 )
    return 1;

  if( current_tick < ticks_in_command ) {
    // handling current command
    current_color.red += step_r;
    current_color.green += step_g;
    current_color.blue += step_b;
    current_tick ++;
  }
  else {
    // set the final color of the transition
    current_color = RgbColorToRgbColor16( &current_command->color );
      
    if( ++ sequence_index < sequence_length ) {
      SetCurrentCommand();
    }
    else {
      sequence_index = 0;
      SetCurrentCommand();
      return 0;
    }
  }

  return 1;
}

int SEQ_CurrentRed()
{
  return ( unsigned char )( current_color.red / 256 ) & 0xFF;
}

int SEQ_CurrentGreen()
{
  return ( unsigned char )( current_color.green / 256 ) & 0xFF;
}

int SEQ_CurrentBlue()
{
  return ( unsigned char )( current_color.blue / 256 ) & 0xFF;
}


