#include <avr/interrupt.h>

#include "logging.h"
#include "keypad.h"
#include "lock.h"

void setup()
{
    loggerInit();
    keypadInit();
    lockInit();
    sei();
}

void loop()
{
    if(hasKeyChanged() != 0){
        handleKeyChange(setlockInput);
    }
    lockRun();
}
