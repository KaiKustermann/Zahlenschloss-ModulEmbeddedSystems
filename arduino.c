#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "logging.h"
#include "enums.h"
#include "keypad.h"

uint8_t volatile flag = 0;

void setup()
{
    keypadInit();
    loggerInit();
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
    uint8_t pressedKey = findPressedKey();
    if(pressedKey != 0){
        char pressedKeyStr[2]; 
        pressedKeyStr[0] = pressedKey; 
        pressedKeyStr[1] = '\0'; 
        logMessage(pressedKeyStr, INFO);
    }
    if (flag)
    {
        PORTB ^= (1 << PB5);
        // logMessage("Hello World", INFO);
        flag = 0;
    }
}

ISR(TIMER1_COMPA_vect)
{
    flag = 1;
}
