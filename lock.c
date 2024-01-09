#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>

#include "logging.h"
#include "lock.h"
#include "stringHelpers.h"
#include "eepromHelpers.h"

#define EEPROM_ADDRESS 0x00
// 15 + null terminator = 16 bytes
#define MAX_PINCODE_LENGTH 15
#define MIN_PINCODE_LENGTH 4

#define PRIMARY_KEY 'A'
#define SECONDARY_KEY 'B'
#define CLEAR_KEY 'C'
#define DELETE_KEY 'D'

const unsigned char pinButtons[] = {'1','2','3','4','5','6','7','8','9','*','0', '#'};

state_t currentState = STATE_INITIAL;
state_t previousState = STATE_INITIAL;
unsigned char lockInput = ' ';
// the pincode that is curently set
char currentPincode[MAX_PINCODE_LENGTH + 1] = "";
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

// returns 1 if the pincode is the same as the set pincode
uint8_t verifyPincode(char* pincode, char* currentPincode){
    if (strCmpConstantTime(pincode, currentPincode) == 0) {
        return 1;
    } else {
        return 0;
    }
}

// function to add non state specific behavior to the keypad
// returns 0 if the input was handled (succeess) and 1 if the input was not handled (error)
uint8_t addDefaultStateBehavior(unsigned char stateInput){
    size_t pincodeLength = strlen(pincode);
    if (stateInput == ' ') {
    }
    else if (stateInput == DELETE_KEY) {
        strDeleteLastCharacter(pincode);
        logMessage(pincode, INFO);
    }
    else if (stateInput == CLEAR_KEY) {
        strClear(pincode);
        logMessage(pincode, INFO);
    }
    else if (pincodeLength == MAX_PINCODE_LENGTH) {
        logMessage("the maximum length of the pincode is reached", INFO);
    }
    else if (isPinButton(stateInput)) {
        pincode[pincodeLength] = stateInput;
        pincode[pincodeLength + 1] = '\0';
        logMessage(pincode, INFO);
    }
    else {
        // Return 1 if the input was not handled (error)
        return 1;
    }
    // Return 0 if the input was handled (success)
    return 0;
}

state_t runStateInitial(unsigned char stateInput, state_t previousState){
    logMessage("welcome", INFO);
    // check if a saved pincode was found in eeprom memory
    if((uint8_t)currentPincode[0] != EEPROM_DEFAULT_VALUE_PER_BYTE){
        return STATE_TRY_PIN_CODE;
    }
    return STATE_SET_PIN_CODE_INITIAL;
};

state_t runStateTryPincode(unsigned char stateInput, state_t previousState){
    if(currentState != previousState){
        logMessage("enter pincode", INFO);
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultStateBehavior(stateInput) == 0){
        return currentState;
    };
    // state change to state set pincode
    if(stateInput == SECONDARY_KEY){
        return STATE_SET_PIN_CODE_SUBSTATE_ENTER_CURRENT;
    }
    size_t pincodeLength = strlen(pincode);
    // if primary key is pressed, try to open the lock
    if(stateInput == PRIMARY_KEY){
        if(pincodeLength < MIN_PINCODE_LENGTH){
            logMessage("the pincode must contain at least 4 characters", INFO);
            return currentState;
        }
        pincode[pincodeLength] = '\0';
        if(verifyPincode(pincode, currentPincode) != 0){
            logMessage("the given pincode was right", INFO);
            return STATE_OPEN;
        } 
        logMessage("the given pincode was wrong", INFO);
        return currentState;
    }
    return currentState;
};

state_t runStateSetPincodeInitial(unsigned char stateInput, state_t previousState){
    if(currentState != previousState){
        logMessage("set a new pincode", INFO);
    }
    // if the input was handeled by default keypad behavior, return
    if (addDefaultStateBehavior(stateInput) == 0){
        return currentState;
    };
    size_t pincodeLength = strlen(pincode);
    // if primary key is pressed, save the pincode in EEPROM
    if(stateInput == PRIMARY_KEY){
        if(pincodeLength < MIN_PINCODE_LENGTH){
            logMessage("the pincode must contain at least 4 characters", INFO);
            return currentState;
        }
        pincode[pincodeLength] = '\0';
        eeprom_write_block((const void*)pincode, (void*)EEPROM_ADDRESS, sizeof(pincode));
        eeprom_read_block((void*)currentPincode, (const void*)EEPROM_ADDRESS, sizeof(currentPincode));
        logMessage("pincode was set", INFO);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
};

// substate of state set pincode, which request the user to enter the current pincode
state_t runStateSetPincodeSubstateEnterCurrent(unsigned char stateInput, state_t previousState){
    if(currentState != previousState){
        logMessage("to set a new pincode, enter the current pincode first", INFO);
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultStateBehavior(stateInput) == 0){
        return currentState;
    };
    if(stateInput == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    size_t pincodeLength = strlen(pincode);
    // if primary key is pressed, try to open the lock
    if(stateInput == PRIMARY_KEY){
        if(pincodeLength < MIN_PINCODE_LENGTH){
            logMessage("the pincode must contain at least 4 characters", INFO);
            return currentState;
        }
        pincode[pincodeLength] = '\0';
        if(verifyPincode(pincode, currentPincode) != 0){
            logMessage("the given pincode was right", INFO);
            return STATE_SET_PIN_CODE_SUBSTATE_ENTER_NEW;
        } 
        logMessage("the given pincode was wrong", INFO);
        return currentState;
    }
    return currentState;
}

// substate of state set pincode, which request the user to enter the new pincode
state_t runStateSetPincodeSubstateEnterNew(unsigned char stateInput, state_t previousState){
    if(currentState != previousState){
        logMessage("set a new pincode", INFO);
    }
    // if the input was handeled by default keypad behavior, return current state
    if (addDefaultStateBehavior(stateInput) == 0){
        return currentState;
    };
    // state change to state try pincode
    if(stateInput == SECONDARY_KEY){
        return STATE_TRY_PIN_CODE;
    }
    size_t pincodeLength = strlen(pincode);
    // if primary key is pressed, save the pincode in EEPROM
    if(stateInput == PRIMARY_KEY){
        if(pincodeLength < MIN_PINCODE_LENGTH){
            logMessage("the pincode must contain at least 4 characters", INFO);
            return currentState;
        }
        pincode[pincodeLength] = '\0';
        eeprom_write_block((const void*)pincode, (void*)EEPROM_ADDRESS, sizeof(pincode));
        eeprom_read_block((void*)currentPincode, (const void*)EEPROM_ADDRESS, sizeof(currentPincode));
        logMessage("pincode was set", INFO);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}

state_t runStateOpen(unsigned char stateInput, state_t previousState){
    if(currentState != previousState){
        logMessage("opening lock", INFO);
        // turn LED on
        PORTB ^= (1 << PB5);
    }
    if(stateInput == SECONDARY_KEY){
        // turn LED off
        PORTB &= ~(1 << PB5);
        return STATE_TRY_PIN_CODE;
    }
    return currentState;
}

state_func_t* const stateTable[NUM_STATES] = {
    runStateInitial, runStateTryPincode, runStateSetPincodeInitial, runStateSetPincodeSubstateEnterCurrent, runStateSetPincodeSubstateEnterNew, runStateOpen
};

state_t runState(state_t currentState, char stateInput, state_t previousState) {
    lockInput = ' ';
    return stateTable[currentState](stateInput, previousState);
};

void lockInit (void) {
    currentState = STATE_INITIAL;
    previousState = STATE_INITIAL;
    lockInput = ' ';
    // read set pincode and write to currentPincode
    eeprom_read_block((void*)currentPincode, (const void*)EEPROM_ADDRESS, sizeof(currentPincode));
    pincode[0] = '\0';
    // set LED pin as output
    DDRB |= 1 << PB5; 
}

void setlockInput(unsigned char input){
    lockInput = input;
}

void handleStateChange(){
    // clear pincode variable bewteen state changes because it is state specific
    pincode[0] = '\0';
}

void lockRun(){
    const state_t previousStateLocal = currentState;
    currentState = runState(currentState, lockInput, previousState);
    previousState = previousStateLocal;
    if(previousState != currentState){
        handleStateChange();
    }
}