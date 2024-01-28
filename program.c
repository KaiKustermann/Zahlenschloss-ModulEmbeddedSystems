#include <avr/interrupt.h>

#include "logging.h"
#include "keypad.h"
#include "lock.h"
#include "lcd.h"
#include "configuration.h"

void setup()
{
    if(ENVIRONMENT == DEV){
        loggingInit();
    }
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
