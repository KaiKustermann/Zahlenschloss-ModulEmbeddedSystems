#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "logging.h"
#include "enums.h"
#include "keypad.h"
#include "uart.h"
#include "lock.h"

uint8_t volatile flag = 0;

void setup()
{
    loggerInit();
    keypadInit();
    lockInit();
    DDRB |= 1 << PB5;    // set PB5 as output (sets to 1)
    DDRB &= ~(1 << PB4); // set PB4 as Input (sets to 0)
    PORTB |= 1 << PB4;   // connect internal pullup for PB4

    TCCR1B |= (1 << WGM12);    // configure timer 1 in CTC mode
    TCCR1B |= (1 << CS12);     // configure 256 prescaler
    OCR1A = (F_CPU / 256) - 1; // count for 1 second with the selected prescaler

    TIMSK1 |= (1 << OCIE1A); // enable timer output compare match interrupt
    sei();
}

void loop()
{
    if(hasKeyChanged() != 0){
        handleKeyChange(setlockInput);
    }
    lockRun();
    if (flag)
    {
        PORTB ^= (1 << PB5);
        flag = 0;
    }
}

ISR(TIMER1_COMPA_vect)
{
    flag = 1;
}
