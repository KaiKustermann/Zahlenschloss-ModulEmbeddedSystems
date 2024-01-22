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
    lockInit();
    sei();
}

void loop()
{
    keypadRun();
    if(hasKeyBeenReleased() != 0){
        const uint8_t pressedKey = getPressedKey();
        const uint32_t timeSinceKeyPressed = getTimeSinceKeyPressInit();
        if(pressedKey != 0){
            setlockInput((unsigned char)pressedKey, timeSinceKeyPressed);
        }
    }
    lockRun();
}
