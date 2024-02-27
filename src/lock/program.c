#include <avr/interrupt.h>

#include "../logging/logging.h"
#include "../drivers/keypad.h"
#include "lock.h"
#include "../drivers/lcd.h"
#include "../configuration.h"

void setup()
{
    // activate logging, if the system is running in a development environment
    // for production, logging is disabled
    if(ENVIRONMENT == DEV){
        loggingInit();
    }
    LCDInit();
    keypadInit();
    // setting the callback function to setLockInput
    setKeyPressHandler(setLockInput);
    lockInit();
    sei();
}

void loop()
{
    keypadRun();
    lockRun();
}
