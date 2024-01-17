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
    keypadRun();
    if(hasKeyBeenReleased() != 0){
        const uint8_t pressedKey = getPressedKey();
        const uint32_t timeSinceKeyPressed = getTimeSinceKeyPressInit();
        logMessageUInt32(timeSinceKeyPressed, INFO);
        if(pressedKey != 0){
            setlockInput((unsigned char)pressedKey, timeSinceKeyPressed);
        }
    }
    lockRun();
}
