#include "delay.h"

#include "system_LPC11xx.h"

void delay_ns(int ns)
{
    volatile unsigned int cycles = ns / (1000000000 / SystemCoreClock);
    while (cycles--);
}

