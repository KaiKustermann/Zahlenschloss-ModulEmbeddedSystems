#include <avr/io.h>
#include <util/atomic.h>
#include <avr/interrupt.h>

#include "timerHelpers.h"

// compare match interrupt gets fired every millisecond 
ISR(TIMER1_COMPA_vect)
{
    timerMillis++;
}

// starts the timer using timer1
void initTimer()
{
    unsigned long CTCMatchOverflow = ((F_CPU / 1000) / 8); //when timer1 is this value, 1ms has passed (with 8 bit prescaler)

    // set timer to clear when matching CTCMatchOverflow, setting prescaler to 8
    TCCR1B |= (1 << WGM12) | (1 << CS11);

    // high byte first, then low byte
    OCR1AH = (CTCMatchOverflow >> 8);
    OCR1AL = CTCMatchOverflow;

    // enable the compare match interrupt
    TIMSK1 |= (1 << OCIE1A);
}

// resets the timer
void resetTimer() {
    // clear the timer control register to stop the timer
    TCCR1B &= ~(1 << CS11);

    // disable the compare match interrupt
    TIMSK1 &= ~(1 << OCIE1A);

    // reset variable
    timerMillis = 0;
}

// returns the milliseconds passed since the timer was started using initTimer()
unsigned long getMillis (void)
{
    unsigned long millisReturn;

    // ensures this cannot be interrupted by other ISRs
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
      millisReturn = timerMillis;
    }

    return millisReturn;
}