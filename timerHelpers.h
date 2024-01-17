#include <avr/io.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef TIMER_HELPERS_H
#define TIMER_HELPERS_H
void initTimer();
void resetTimer();
uint32_t millis();    
#endif 

#ifdef __cplusplus
}
#endif