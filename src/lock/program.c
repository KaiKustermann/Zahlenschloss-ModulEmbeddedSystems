#include <avr/interrupt.h>

#include "../logging/logging.h"
#include "../drivers/keypad.h"
#include "lock.h"
#include "../drivers/lcd.h"
#include "../configuration.h"

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
