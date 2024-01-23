#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>

#include "logging.h"
#include "lock.h"
#include "stringHelpers.h"
#include "eepromHelpers.h"
#include "hashing.h"
#include "lcd.h"


#define EEPROM_ADDRESS_HASHING_SALT 0x00
#define HASHING_SALT_SIZE 8U
// EEPROM_ADDRESS_PINCODE depends on salt size, as this is also saved to EEPROM
#define EEPROM_ADDRESS_SAVED_PINCODE (EEPROM_ADDRESS_HASHING_SALT + HASHING_SALT_SIZE)
#define SAVED_PINCODE_SIZE 10U
// 15 + null terminator = 16
#define MAX_PINCODE_LENGTH 15
#define MIN_PINCODE_LENGTH 4

#define PRIMARY_KEY 'A'
#define SECONDARY_KEY 'B'
#define CLEAR_KEY 'C'
#define DELETE_KEY 'D'
#define PRESS_DURATION_RESET 4000UL

const unsigned char pinButtons[] = {'1','2','3','4','5','6','7','8','9','*','0', '#'};

state_t currentState = STATE_INITIAL;
state_t previousState = STATE_INITIAL;
unsigned char lockKeyInput = ' ';
uint32_t lockKeyPressDuration = 0;

// is used as temporary state specific variable, cleared on state change
char pincode[MAX_PINCODE_LENGTH + 1] = "";

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

void saveSalt(char* salt){
    eeprom_write_block((const void*)salt, (void*)EEPROM_ADDRESS_HASHING_SALT, (size_t)HASHING_SALT_SIZE);
}

void getSavedSalt(char* dest){
    eeprom_read_block((void*)dest, (const void*)EEPROM_ADDRESS_HASHING_SALT, (size_t)HASHING_SALT_SIZE);
}

// reads the saved pincode from EEPROM and saves it in dest
void getSavedPincode(char* dest){
    eeprom_read_block((void*)dest, (const void*)EEPROM_ADDRESS_SAVED_PINCODE, SAVED_PINCODE_SIZE);
}

// saves pincode to EEPROM and hashes it beforehand
void savePincode(char* pincode){
    char salt[HASHING_SALT_SIZE];
    generateSalt(salt, sizeof(salt));
    char hashedPincodeTemp[SAVED_PINCODE_SIZE];
    hashPincode(pincode, hashedPincodeTemp, sizeof(hashedPincodeTemp), salt);
    logMessage("saving pincode...", INFO);
    saveSalt(salt);
    eeprom_write_block((const void*)hashedPincodeTemp, (void*)EEPROM_ADDRESS_SAVED_PINCODE, sizeof(hashedPincodeTemp));
    logMessage("pincode saved!", INFO);
}

// returns 1 if the pincode is the same as the saved pincode
uint8_t verifyPincode(char* pincode){
    // hashing pincode before comparing
    char salt[HASHING_SALT_SIZE];
    getSavedSalt(salt);
    char hashedPincodeTemp[SAVED_PINCODE_SIZE];
    hashPincode(pincode, hashedPincodeTemp, sizeof(hashedPincodeTemp), salt);
    char savedHashedPincodeTemp[SAVED_PINCODE_SIZE];
    getSavedPincode(savedHashedPincodeTemp);
    logMessage("verifying pincode...", INFO);
    if (strCmpConstantTime(hashedPincodeTemp, savedHashedPincodeTemp) == 0) {
        logMessage("pincode is correct!", INFO);
        return 1;
    } else {
        logMessage("pincode is incorrect!", INFO);
        return 0;
    }
}

void lockInit (void) {
    currentState = STATE_INITIAL;
    previousState = STATE_INITIAL;
    lockKeyInput = ' ';
    lockKeyPressDuration = 0;
    // reset temporary pincode variable that lives as long as a state
    pincode[0] = '\0';
    // set LED pin as output
    DDRB |= 1 << PB5; 
}

void lockReset(){
    // turn off LED if it was on
    PORTB &= ~(1 << PB5);
    // clear EEPROM
    eepromReset();
    // reinitialize the lock system
    lockInit();
}

void setlockInput(unsigned char keyInput, uint32_t keyPressDuration){
    lockKeyInput = keyInput;
    lockKeyPressDuration = keyPressDuration;
}

void handleGenericStateChange(){
    // clear pincode variable bewteen state changes because it is state specific
    pincode[0] = '\0';
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
        LCDclearLastInput();
    }
    else if (lockKeyInput == CLEAR_KEY) {
        strClear(pincode);
        logMessage(pincode, INFO);
        LCDclearRow(2);
    }
    else if (lockKeyInput == PRIMARY_KEY && pincodeLength < MIN_PINCODE_LENGTH){
        logMessage("the pincode must contain at least 4 characters", INFO);
    }
    else if (isPinButton(lockKeyInput) && (pincodeLength == MAX_PINCODE_LENGTH)) {
        logMessage("the maximum length of the pincode is reached", INFO);
    }
    else if (isPinButton(lockKeyInput)) {
        pincode[pincodeLength] = lockKeyInput;
        pincode[pincodeLength + 1] = '\0';
        logMessage(pincode, INFO);
        LCDsend8Bit('*', 1);
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
    if(lockKeyInput == CLEAR_KEY && lockKeyPressDuration > 5000){
        return 1;
    }
    return 0;
}

state_t runStateInitial(){
    logMessage("welcome! ", INFO);
    // check if a saved pincode was found in EEPROM memory
    char savedPincode[SAVED_PINCODE_SIZE];
    getSavedPincode(savedPincode);
    if((uint8_t)savedPincode[0] != EEPROM_DEFAULT_VALUE_PER_BYTE){
        return STATE_TRY_PIN_CODE;
    }
    return STATE_SET_PIN_CODE_INITIAL;
};

state_t runStateTryPincode(){
    if(currentState != previousState){
        logMessage("enter pincode ", INFO);
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
        return currentState;
    }
    return currentState;
};

state_t runStateSetPincodeInitial(){
    if(currentState != previousState){
        logMessage("set a new pincode", INFO);
        LCDclear();
        LCDWriteString("Enter new PIN:");
        LCDSetCursorPosition(0, 1);
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

// substate of state set pincode, which request the user to enter the current pincode
state_t runStateSetPincodeSubstateEnterCurrent(){
    if(currentState != previousState){
        logMessage("to set a new pincode, enter the current pincode first ", INFO);
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultKeypadBehavior() == 0){
        return currentState;
    }
    if(lockKeyInput == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    // if primary key is pressed, try to open the lock
    if(lockKeyInput == PRIMARY_KEY){
        if(verifyPincode(pincode) != 0){
            return STATE_SET_PIN_CODE_SUBSTATE_ENTER_NEW;
        }
        strClear(pincode);
        return currentState;
    }
    return currentState;
}

// substate of state set pincode, which request the user to enter the new pincode
state_t runStateSetPincodeSubstateEnterNew(){
    if(currentState != previousState){
        logMessage("set a new pincode ", INFO);
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
        logMessage("opening lock ", INFO);
        // turn LED on
        PORTB ^= (1 << PB5);
    }
    if(lockKeyInput == SECONDARY_KEY){
        // turn LED off
        PORTB &= ~(1 << PB5);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}


state_t runStateReset(){
    if(currentState != previousState){
        logMessage("enter the current pincode to complete reset ", INFO);
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
            logMessage("resetting lock...", INFO);
            lockReset();
            logMessage("reset succeeded!", INFO);
            return STATE_INITIAL;
        }
        strClear(pincode);
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
    if(isResetSystemInitiated() != 0){
        return STATE_RESET;
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