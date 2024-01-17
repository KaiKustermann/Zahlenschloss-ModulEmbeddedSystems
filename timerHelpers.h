#include <avr/io.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TIMER_HELPERS_H
#define TIMER_HELPERS_H
volatile unsigned long timerMillis;
ISR(TIMER1_COMPA_vect);
void initMillis();
unsigned long millis (void);
#endif 

#ifdef __cplusplus
}
#endif