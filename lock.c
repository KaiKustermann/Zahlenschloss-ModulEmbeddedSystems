#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>
#include <util/delay.h>

#include "logging.h"
#include "lock.h"
#include "stringHelpers.h"
#include "eepromHelpers.h"
#include "hashing.h"
#include "lcd.h"


#define EEPROM_ADDRESS_HASHING_SALT 0x00
#define HASHING_SALT_SIZE 8U
#define EEPROM_ADDRESS_SAVED_PINCODE (EEPROM_ADDRESS_HASHING_SALT + HASHING_SALT_SIZE) // depends on salt size, as this is also saved to EEPROM
#define SAVED_PINCODE_SIZE 10U // the length of the hashed and saved pincode
#define MAX_PINCODE_LENGTH 16 // 16 + null terminator = 17
#define MIN_PINCODE_LENGTH 4

#define PRIMARY_KEY 'A'
#define SECONDARY_KEY 'B'
#define CLEAR_KEY 'C'
#define DELETE_KEY 'D'
#define PINCODE_MASK_KEY '*'
#define PRESS_DURATION_RESET 4000UL

#define HELP_MESSAGE_SCREEN_TIME 1000 // how many ms messages of type help message are displayed on the screen


const unsigned char pinButtons[] = {'1','2','3','4','5','6','7','8','9','0', '#'}; // which buttons can be used for entering the pin

state_t currentState = STATE_INITIAL;
state_t previousState = STATE_INITIAL;
unsigned char lockKeyInput = ' ';
uint32_t lockKeyPressDuration = 0;

char pincode[MAX_PINCODE_LENGTH + 1] = ""; // is used as temporary state specific variable, cleared on state change
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

// returns 0 if displayed pincode is not masked, a non zero value if it is/should be masked
uint8_t isDisplayedPincodeMasked(){
    return displayedPincodeMasked;
}

// masks the pincode for displaying it on the screen
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

// writes pincode to second row of the screen, determines if it should be masked or not using the current state
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

// writes help message to second row of the screen, that is there for one second
void writeHelpMessageToScreen(char* helpMessage){
    LCDOverwriteStringRowTwo(helpMessage);
    _delay_ms(1000);
    writePincodeToScreen(pincode);
    LCDSetCursorPosition((unsigned char)strlen(pincode), 1);
}

// writes custom state message to first row of the screen
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

// saves pincode to EEPROM and hashes it beforehand with random salt
void savePincode(char* pincode){
    char salt[HASHING_SALT_SIZE];
    generateSalt(salt, sizeof(salt));
    char hashedPincodeTemp[SAVED_PINCODE_SIZE];
    hashPincode(pincode, hashedPincodeTemp, sizeof(hashedPincodeTemp), salt);
    logMessage("saving pincode...", INFO);
    writeHelpMessageToScreen("Saving...");
    saveSalt(salt);
    eeprom_write_block((const void*)hashedPincodeTemp, (void*)EEPROM_ADDRESS_SAVED_PINCODE, sizeof(hashedPincodeTemp));
    logMessage("pincode saved!", INFO);
    writeHelpMessageToScreen("Saved!");
}

// returns 1 if the pincode is the same as the saved pincode
uint8_t verifyPincode(char* pincode){
    char salt[HASHING_SALT_SIZE];
    getSavedSalt(salt);
    char hashedPincodeTemp[SAVED_PINCODE_SIZE];
    hashPincode(pincode, hashedPincodeTemp, sizeof(hashedPincodeTemp), salt); // hashing pincode with saved salt before comparing
    char savedHashedPincodeTemp[SAVED_PINCODE_SIZE];
    getSavedPincode(savedHashedPincodeTemp);
    logMessage("verifying pincode...", INFO);
    writeHelpMessageToScreen("Verifying...");
    if (strCmpConstantTime(hashedPincodeTemp, savedHashedPincodeTemp) == 0) { // comparing with constant time respective to the saved pincode
        logMessage("pincode is correct!", INFO);
        writeHelpMessageToScreen("Pin correct!");
        return 1;
    } else {
        logMessage("pincode is incorrect!", INFO);
        writeHelpMessageToScreen("Pin incorrect!");
        return 0;
    }
}

// initializes the lock with default settings
void lockInit (void) {
    currentState = STATE_INITIAL;
    previousState = STATE_INITIAL;
    lockKeyInput = ' ';
    lockKeyPressDuration = 0;
    displayedPincodeMasked = 1; // display the pincode as masked by default
    // reset temporary pincode variable that lives as long as a state
    strClear(pincode);
    // set LED pin as output
    DDRB |= 1 << PB5; 
}

// resets the lock to factory state
void lockReset(){
    logMessage("resetting lock...", INFO);
    writeHelpMessageToScreen("Reset started!");
    // turn off LED if it was on
    PORTB &= ~(1 << PB5);
    // clear EEPROM
    eepromReset();
    // reinitialize the lock system
    lockInit();
    logMessage("reset succeeded!", INFO);
    writeHelpMessageToScreen("Succeeded!");
}

// sets the input to the lock
void setlockInput(unsigned char keyInput, uint32_t keyPressDuration){
    lockKeyInput = keyInput;
    lockKeyPressDuration = keyPressDuration;
}

void handleGenericStateChange(){
    // clear pincode variable bewteen state changes because it is state specific
    strClear(pincode);
    // write it to the screen
    writePincodeToScreen(pincode);
}

// adds non state specific behavior to the keypad
// returns 0 if the input was handled (succeess) and 1 if the input was not handled (error)
uint8_t addDefaultKeypadBehavior(){
    size_t pincodeLength = strlen(pincode);
    if (lockKeyInput == ' ') {
    }
    else if (lockKeyInput == DELETE_KEY) {
        strDeleteLastCharacter(pincode);
        logMessage(pincode, INFO);
        writePincodeToScreen(pincode);
    }
    else if (lockKeyInput == CLEAR_KEY) {
        strClear(pincode);
        logMessage(pincode, INFO);
        writePincodeToScreen(pincode);
    }
    else if (lockKeyInput == PRIMARY_KEY && pincodeLength < MIN_PINCODE_LENGTH){
        strClear(pincode);
        logMessage("the pincode must contain at least 4 characters", INFO);
        writeHelpMessageToScreen("Min 4 digits!");
    }
    else if (isPinButton(lockKeyInput) && (pincodeLength == MAX_PINCODE_LENGTH)) {
        strClear(pincode);
        logMessage("the maximum length of the pincode is reached", INFO);
        writeHelpMessageToScreen("Max 16 digits!");
    }
    else if (lockKeyInput == PINCODE_MASK_KEY){
        if(isDisplayedPincodeMasked() != 0){
            displayedPincodeMasked = 0;
        } else {
            displayedPincodeMasked = 1;
        }
        logMessage("pincode mask key pressed", INFO);
        writePincodeToScreen(pincode);
    }
    else if (isPinButton(lockKeyInput)) {
        pincode[pincodeLength] = lockKeyInput;
        pincode[pincodeLength + 1] = '\0';
        logMessage(pincode, INFO);
        writePincodeToScreen(pincode);
    }
    else {
        // Return 1 if the input was not handled (error)
        return 1;
    }
    // Return 0 if the input was handled (success)
    return 0;
}

// checks if the user initiated a system reset
// returns a non zero value if it is initiated and 0 if no reset is initiated
uint8_t isResetSystemInitiated(){
    if(lockKeyInput == CLEAR_KEY && lockKeyPressDuration > PRESS_DURATION_RESET){
        return 1;
    }
    return 0;
}

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

state_t runStateSetPincodeInitial(){
    if(currentState != previousState){
        logMessage("entered state set pincode initial", INFO);
        writeStateMessageToScreen("Set new Pincode:");
    }
    // if the input was handeled by default keypad behavior, return
    if (addDefaultKeypadBehavior() == 0){
        return currentState;
    }
    // if primary key is pressed, save the pincode in EEPROM
    if(lockKeyInput == PRIMARY_KEY){
        savePincode(pincode);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
};

state_t runStateTryPincode(){
    if(currentState != previousState){
        logMessage("entered state try pincode ", INFO);
        writeStateMessageToScreen("Enter Pincode:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultKeypadBehavior() == 0){
        return currentState;
    }
    // state change to state set pincode
    if(lockKeyInput == SECONDARY_KEY){
        return STATE_SET_PIN_CODE_SUBSTATE_ENTER_CURRENT;
    }
    // if primary key is pressed, try to open the lock
    if(lockKeyInput == PRIMARY_KEY){
        if(verifyPincode(pincode) != 0){
            return STATE_OPEN;
        }
        strClear(pincode);
        writePincodeToScreen(pincode);
        return currentState;
    }
    return currentState;
};

// substate of state set pincode, which request the user to enter the current pincode
state_t runStateSetPincodeSubstateEnterCurrent(){
    if(currentState != previousState){
        logMessage("entered state set pincode substate enter current", INFO);
        writeStateMessageToScreen("To set new...");
        _delay_ms(1000);
        writeStateMessageToScreen("Enter current:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultKeypadBehavior() == 0){
        return currentState;
    }
    if(lockKeyInput == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, enter state set pincode substate set new
    if(lockKeyInput == PRIMARY_KEY){
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
    if (addDefaultKeypadBehavior() == 0){
        return currentState;
    }
    // state change to state try pincode
    if(lockKeyInput == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, save the pincode in EEPROM
    if(lockKeyInput == PRIMARY_KEY){
        savePincode(pincode);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}

state_t runStateOpen(){
    if(currentState != previousState){
        logMessage("entering state open", INFO);
        writeStateMessageToScreen("Lock Open!");
        // turn LED on
        PORTB ^= (1 << PB5);
    }
    if(lockKeyInput == SECONDARY_KEY){
        writeHelpMessageToScreen("Closing lock!");
        // turn LED off
        PORTB &= ~(1 << PB5);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}


state_t runStateReset(){
    if(currentState != previousState){
        logMessage("entering state reset", INFO);
        writeStateMessageToScreen("To reset...");
        _delay_ms(1000);
        writeStateMessageToScreen("Enter current:");
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultKeypadBehavior() == 0){
        return currentState;
    }
    // cancel reset if secondary key is pressed
    if(lockKeyInput == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, verify entered pincode
    if(lockKeyInput == PRIMARY_KEY){
        if(verifyPincode(pincode) != 0){
            strClear(pincode);
            writeStateMessageToScreen("Resetting...");
            lockReset();
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

state_t runState() {
    // system reset should not be available during setup
    if(currentState != STATE_INITIAL && currentState != STATE_SET_PIN_CODE_INITIAL){
        if(isResetSystemInitiated() != 0){
            return STATE_RESET;
        }
    }
    return stateTable[currentState]();
};

void lockRun(){
    const state_t previousStateLocal = currentState;
    currentState = runState(currentState, lockKeyInput, lockKeyPressDuration, previousState);
    previousState = previousStateLocal;
    lockKeyInput = ' ';
    lockKeyPressDuration = 0;
    if(previousState != currentState){
        handleGenericStateChange();
    }
}