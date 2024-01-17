#include <avr/io.h>
#include <util/delay.h>

void initTimer() {
    // initialize Timer1 (16 bit timer) in normal mode with 1024 as prescaler (up to 4 seconds until timer overflow)
    TCCR1B |= (1 << CS10);
}

void resetTimer() {
    // clear the bits responsible for prescaler to stop the timer
    TCCR1B &= ~(1 << CS10);
    // reset the timer value to 1
    TCNT1 = 0x0001;
}

uint32_t millis() {
    // Timer1 is a 16-bit timer, and the clock frequency is 1 MHz
    return (TCNT1 * 1000UL) / 1000;
}