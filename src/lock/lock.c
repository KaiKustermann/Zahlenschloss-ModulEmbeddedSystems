#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "../logging/logging.h"
#include "lock.h"
#include "../helpers/stringHelpers.h"
#include "../helpers/eepromHelpers.h"
#include "hashing.h"
#include "../drivers/lcd.h"
#include "../drivers/buzzer.h"


#define EEPROM_ADDRESS_HASHING_SALT 0x00
#define HASHING_SALT_SIZE 8U // size of the salt, which the pin code is hashed with
#define EEPROM_ADDRESS_SAVED_PINCODE (EEPROM_ADDRESS_HASHING_SALT + HASHING_SALT_SIZE) // depends on salt size, as this is also saved to EEPROM
#define SAVED_PINCODE_SIZE 10U // the length of the hashed and saved pincode
#define MAX_PINCODE_LENGTH 16 // maximum length of the saveable pin code
#define MIN_PINCODE_LENGTH 4 // maximum length of the saveable pin code

#define PRIMARY_KEY 'A'
#define SECONDARY_KEY 'B'
#define CLEAR_KEY 'C'
#define DELETE_KEY 'D'
#define PINCODE_MASK_KEY '*'
#define RESET_KEY '#'
#define PRESS_DURATION_RESET 4000UL // how long the reset button should be pressed for a reset to be initialized

#define LED_GREEN PD2
#define LED_RED PD3
#define LED_DDR DDRD
#define LED_PORT PORTD

#define HELP_MESSAGE_SCREEN_TIME 1000 // how many milliseconds messages of type help message are displayed on the screen


const unsigned char pinButtons[] = {'1','2','3','4','5','6','7','8','9','0'}; // which buttons can be used for entering the pin

state_t currentState = STATE_INITIAL;
state_t previousState = STATE_INITIAL;
struct keyEvent lockInput; 

char pincode[MAX_PINCODE_LENGTH + 1] = ""; // is used as temporary state specific variable, cleared on state change (represents the displayed pin)
uint8_t displayedPincodeMasked = 1; // 0 if displayed pincode is not masked, non zero if it is (keeping track for toggle)


// takes a char as parameter and checks if it is a pin button (e.g. possible pin value)
uint8_t isPinButton(unsigned char button){
    int pinButtonsArrayLength = sizeof(pinButtons)/sizeof(pinButtons[0]);
    for(int i = 0; i<pinButtonsArrayLength; i++){
        if(pinButtons[i] == button){
            return 1;
        }
    }
    return 0;
}

// returns 0 if displayed pincode is not masked, a non zero value if it is masked
uint8_t isDisplayedPincodeMasked(){
    return displayedPincodeMasked;
}

// masks the pincode for displaying it on the screen (masks the first chars with asterisk chars, e.g. 1234 -> ***4)
void maskPincode(const char* pincode, char* maskedPincode, size_t lenPincode) {
    for (uint8_t i = 0; i < lenPincode; i++) {
        if (i == (lenPincode - 1)) {
            maskedPincode[i] = pincode[i];
            break;
        }
        maskedPincode[i] = '*';
    }
    maskedPincode[lenPincode] = '\0';
}

// writes the pincode to the second row of the screen, determines if it should be masked or not using the current user setting
void writePincodeToScreen(char* pincode){
    size_t lenPincode = strlen(pincode);
    char pincodeToDisplay[MAX_PINCODE_LENGTH + 1]; // +1 for null terminator
    if(isDisplayedPincodeMasked() != 0){ // checks if the pincode should be masked
        maskPincode(pincode, pincodeToDisplay, lenPincode);
    } else {
        strcpy(pincodeToDisplay, pincode);
    }
    LCDOverwriteStringRowTwo(pincodeToDisplay);
    LCDSetCursorPosition((unsigned char)strlen(pincode), 1);
}

// writes a given help message to the second row of the screen, which is displayed for one second
// takes playTone as second argument, where 0 = no tone, 1 = high tone, 2 = low tone
void writeHelpMessageToScreen(char* helpMessage, int playTone){
    LCDOverwriteStringRowTwo(helpMessage);
    if(playTone == 0) {
        _delay_ms(HELP_MESSAGE_SCREEN_TIME);
    } else if(playTone == 1){
        playToneHigh();
        _delay_ms(100);
        playToneHigher();
    } else if(playTone == 2){
        playToneLow();
    }
    LCDSetCursorPosition((unsigned char)strlen(pincode), 1);
}

// writes the given message to first row of the screen, which should give information about the current state of the system
void writeStateMessageToScreen(char* stateMessage){
    LCDOverwriteStringRowOne(stateMessage);
    LCDSetCursorPosition((unsigned char)strlen(pincode), 1);
}

// saves pin salt to eeprom
void saveSalt(char* salt){
    eeprom_write_block((const void*)salt, (void*)EEPROM_ADDRESS_HASHING_SALT, (size_t)HASHING_SALT_SIZE);
}

// retrieves pin salt from eeprom
void getSavedSalt(char* dest){
    eeprom_read_block((void*)dest, (const void*)EEPROM_ADDRESS_HASHING_SALT, (size_t)HASHING_SALT_SIZE);
}

// reads the saved pincode from EEPROM and saves it in dest
void getSavedPincode(char* dest){
    eeprom_read_block((void*)dest, (const void*)EEPROM_ADDRESS_SAVED_PINCODE, SAVED_PINCODE_SIZE);
}

// saves pincode to EEPROM and hashes it beforehand with random salt, displays help messages to inform the user
void savePincode(char* pincode){
    char salt[HASHING_SALT_SIZE];
    generateSalt(salt, sizeof(salt));
    char hashedPincodeTemp[SAVED_PINCODE_SIZE];
    hashPincode(pincode, hashedPincodeTemp, sizeof(hashedPincodeTemp), salt);
    logMessage("saving pincode...", INFO);
    writeHelpMessageToScreen("Saving...", 0);
    saveSalt(salt);
    eeprom_write_block((const void*)hashedPincodeTemp, (void*)EEPROM_ADDRESS_SAVED_PINCODE, sizeof(hashedPincodeTemp));
    logMessage("pincode saved!", INFO);
    writeHelpMessageToScreen("Saved!", 0);
}

// returns 1 if the pincode is the same as the currently set pincode, otherwise returns 0
uint8_t verifyPincode(char* pincode){
    char salt[HASHING_SALT_SIZE];
    getSavedSalt(salt);
    char hashedPincodeTemp[SAVED_PINCODE_SIZE];
    hashPincode(pincode, hashedPincodeTemp, sizeof(hashedPincodeTemp), salt); // hashing pincode with saved salt before comparing
    char savedHashedPincodeTemp[SAVED_PINCODE_SIZE];
    getSavedPincode(savedHashedPincodeTemp);
    logMessage("verifying pincode...", INFO);
    writeHelpMessageToScreen("Verifying...", 0);
    if (strCmpConstantTime(hashedPincodeTemp, savedHashedPincodeTemp) == 0) { // comparing with constant time respective to the saved pincode
        logMessage("pincode is correct!", INFO);
        writeHelpMessageToScreen("Pin correct!", 1);
        return 1;
    } else {
        logMessage("pincode is incorrect!", INFO);
        // blink red LED once
        LED_PORT |= (1 << LED_RED); // turn on red LED
        writeHelpMessageToScreen("Pin incorrect!", 2);
        //playTone();
        _delay_ms(500); // wait for 200 milliseconds
        LED_PORT &= ~(1 << LED_RED); // turn off red LED
        return 0;
    }
}

// initializes the lock with default settings
void lockInit (void) {
    currentState = STATE_INITIAL;
    previousState = STATE_INITIAL;
    lockInput.pressEventType = KEY_NONE;
    displayedPincodeMasked = 1; // display the pincode as masked by default
    // reset temporary pincode variable that lives as long as a state
    strClear(pincode);
    // set LED pin as output
    LED_DDR |= (1 << LED_GREEN) | (1 << LED_RED); // Set PIND4 and PIND5 as output
    init_speaker();
}

// resets the lock to factory settings
void lockReset(){
    logMessage("resetting lock...", INFO);
    writeHelpMessageToScreen("Reset started!", 0);
    // turn off LED if it was on
    LED_PORT &= ~((1 << LED_GREEN) | (1 << LED_RED));
    // clear EEPROM
    eepromReset();
    // reinitialize the lock system
    lockInit();
    logMessage("reset succeeded!", INFO);
    writeHelpMessageToScreen("Succeeded!", 0);
}

// sets the input of the lock
void setLockInput(struct keyEvent keyPressEvent){
    lockInput = keyPressEvent;
}

// handles a state change by clearing the pincode variable
void handleGenericStateChange(){
    // clear pincode variable bewteen state changes because it is state specific
    strClear(pincode);
    // write it to the screen
    writePincodeToScreen(pincode);
}

// adds state independent behavior to the lock by processing state independent input
// returns 0 if the input was handled (succeess) and 1 if the input was not handled (exception)
uint8_t addDefaultLockBehavior(){
    size_t pincodeLength = strlen(pincode);
    if (lockInput.pressEventType == KEY_NONE) {
    }
    else if (lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == DELETE_KEY) {
        strDeleteLastCharacter(pincode);
        logMessage(pincode, INFO);
        writePincodeToScreen(pincode);
    }
    else if (lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == CLEAR_KEY) {
        strClear(pincode);
        logMessage(pincode, INFO);
        writePincodeToScreen(pincode);
    }
    else if (lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PRIMARY_KEY && pincodeLength < MIN_PINCODE_LENGTH){
        strClear(pincode);
        logMessage("the pincode must contain at least 4 characters", INFO);
        writeHelpMessageToScreen("Min 4 digits!", 0);
    }
    else if (lockInput.pressEventType == KEY_PRESS_START && isPinButton(lockInput.pressedKey) && (pincodeLength == MAX_PINCODE_LENGTH)) {
        strClear(pincode);
        logMessage("the maximum length of the pincode is reached", INFO);
        writeHelpMessageToScreen("Max 16 digits!", 0);
    }
    else if (lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PINCODE_MASK_KEY){
        if(isDisplayedPincodeMasked() != 0){
            displayedPincodeMasked = 0;
        } else {
            displayedPincodeMasked = 1;
        }
        logMessage("pincode mask key pressed", INFO);
        writePincodeToScreen(pincode);
    }
    else if (lockInput.pressEventType == KEY_PRESS_START && isPinButton(lockInput.pressedKey)) {
        playToneHigh();
        pincode[pincodeLength] = lockInput.pressedKey;
        pincode[pincodeLength + 1] = '\0';
        logMessage(pincode, INFO);
        writePincodeToScreen(pincode);
    }
    else {
        // return 1 if the input was not handled (exception)
        return 1;
    }
    // return 0 if the input was handled (success)
    return 0;
}

// runs the state initial, which determines if a pin code is currently set or not and changes state accordingly
state_t runStateInitial(){
    logMessage("entered state initial", INFO);
    writeStateMessageToScreen("Welcome!");
    // check if a saved pincode was found in EEPROM memory
    char savedPincode[SAVED_PINCODE_SIZE];
    getSavedPincode(savedPincode);
    if((uint8_t)savedPincode[0] != EEPROM_DEFAULT_VALUE_PER_BYTE){
        return STATE_TRY_PIN_CODE;
    }
    return STATE_SET_PIN_CODE_INITIAL;
};

// runs the state set pincode initial, which represents the inital setting of the pincode
// when using the lock for the first time or after resetting the lock
state_t runStateSetPincodeInitial(){
    if(currentState != previousState){
        logMessage("entered state set pincode initial", INFO);
        writeStateMessageToScreen("Set new Pincode:");
    }
    // if the input was handeled by default keypad behavior, return
    if (addDefaultLockBehavior() == 0){
        return currentState;
    }
    // if primary key is pressed, save the pincode in EEPROM
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PRIMARY_KEY){
        savePincode(pincode);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
};

// runs the state try pincode, which prompts the user to enter the currently set pincode to open the lock
state_t runStateTryPincode(){
    if(currentState != previousState){
        logMessage("entered state try pincode ", INFO);
        writeStateMessageToScreen("Enter Pincode:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultLockBehavior() == 0){
        return currentState;
    }
    // state change to state set pincode, if the secondary key is pressed
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == SECONDARY_KEY){
        return STATE_SET_PIN_CODE_SUBSTATE_ENTER_CURRENT;
    }
    // if primary key is pressed, try to open the lock
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PRIMARY_KEY){
        if(verifyPincode(pincode) != 0){
            return STATE_OPEN;
        }
        strClear(pincode);
        writePincodeToScreen(pincode);
        return currentState;
    }
    return currentState;
};

// substate of state set pincode, which request the user to enter the current pincode first to proceed
state_t runStateSetPincodeSubstateEnterCurrent(){
    if(currentState != previousState){
        logMessage("entered state set pincode substate enter current", INFO);
        writeStateMessageToScreen("To set new...");
        _delay_ms(1000);
        writeStateMessageToScreen("Enter current:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultLockBehavior() == 0){
        return currentState;
    }
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, enter state set pincode substate set new
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PRIMARY_KEY){
        if(verifyPincode(pincode) != 0){
            return STATE_SET_PIN_CODE_SUBSTATE_ENTER_NEW;
        }
        strClear(pincode);
        writePincodeToScreen(pincode);
        return currentState;
    }
    return currentState;
}

// substate of state set pincode, which request the user to enter the new pincode
state_t runStateSetPincodeSubstateEnterNew(){
    if(currentState != previousState){
        logMessage("entered state set pincode substate set new", INFO);
        writeStateMessageToScreen("Enter new Pin:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultLockBehavior() == 0){
        return currentState;
    }
    // state change to state try pincode
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, save the pincode in EEPROM
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PRIMARY_KEY){
        savePincode(pincode);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}

// runs the state open, which opens the lock
state_t runStateOpen(){
    if(currentState != previousState){
        logMessage("entering state open", INFO);
        writeStateMessageToScreen("Lock Open!");
        // turn LED on
        LED_PORT |= (1<< LED_GREEN);
    }
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == SECONDARY_KEY){
        writeHelpMessageToScreen("Closing lock!", 0);
        // turn LED off
        LED_PORT &= ~(1 << LED_GREEN);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}

// runs the state reset, which prompts the user to enter the currently set pincode first
state_t runStateReset(){
    if(currentState != previousState){
        logMessage("entering state reset", INFO);
        writeStateMessageToScreen("To reset...");
        _delay_ms(1000);
        writeStateMessageToScreen("Enter current:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultLockBehavior() == 0){
        return currentState;
    }
    // cancel reset if secondary key is pressed
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, verify entered pincode
    if(lockInput.pressEventType == KEY_PRESS_START && lockInput.pressedKey == PRIMARY_KEY){
        if(verifyPincode(pincode) != 0){
            strClear(pincode);
            writeStateMessageToScreen("Resetting...");
            lockReset(); // if the given pincode was right, reset the lock
            return STATE_INITIAL;
        }
        strClear(pincode);
        writePincodeToScreen(pincode);
        return currentState;
    }
    return currentState;
}

state_func_t* const stateTable[NUM_STATES] = {
    runStateInitial, 
    runStateTryPincode, 
    runStateSetPincodeInitial, 
    runStateSetPincodeSubstateEnterCurrent, 
    runStateSetPincodeSubstateEnterNew, 
    runStateOpen, 
    runStateReset
};

// checks if the user has initiated a system reset
// returns 1 if a reset was initiated and 0 if no reset was initiated
uint8_t isSystemResetInitiated(){
    // system reset should not be available during setup and if the lock is already in state reset
    if(currentState != STATE_INITIAL && currentState != STATE_SET_PIN_CODE_INITIAL && currentState != STATE_RESET){
        // reset is initiated on key hold of RESET_KEY with a longer press duration than PRESS_DURATION_RESET
        if(lockInput.pressEventType == KEY_HOLD && lockInput.pressedKey == RESET_KEY && lockInput.pressDuration > PRESS_DURATION_RESET){
            return 1;
        }
    }
    return 0;
}

// runs the current state
state_t runState() {
    if(isSystemResetInitiated() != 0){
        return STATE_RESET;
    }
    return stateTable[currentState]();
};

// runs the lock by running the current state and determining the next state
void lockRun(){
    const state_t previousStateLocal = currentState;
    currentState = runState();
    previousState = previousStateLocal;
    if(previousState != currentState){
        handleGenericStateChange();
    }
}