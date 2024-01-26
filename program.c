#include <avr/interrupt.h>

#include "logging.h"
#include "keypad.h"
#include "lock.h"
#include "lcd.h"

void setup()
{
    loggerInit();
    LCDInit();
    keypadInit();
    setKeyPressHandler(setLockInput);
    lockInit();
    sei();
}

void loop()
{
    keypadRun();
    lockRun();
}
