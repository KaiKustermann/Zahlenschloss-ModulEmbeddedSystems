#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <string.h>

#include "logging.h"
#include "lock.h"
#include "uart.h"


state_t currentState;
unsigned char lockInput;
uint8_t maximumPinCodeLength = 20;

state_t runStateInitial(unsigned char stateInput){
    logMessage("state initial", INFO);
    return STATE_SET_PIN_CODE;
};
state_t runStateTryPincode(unsigned char stateInput){
    usartPutChar(stateInput);
    logMessage("state try pincode", INFO);
    return STATE_TRY_PIN_CODE;
};
state_t runStateSetPincode(unsigned char stateInput){
    char pincode[maximumPinCodeLength + 1];
    size_t currentPincodeLength = strlen(pincode);
    pincode[currentPincodeLength] = stateInput;
    // originalString[strlen(pincode)] = '\0';
    logMessage("state set pincode", INFO);
    if(stateInput == 'A'){
        return STATE_TRY_PIN_CODE;
    }
    return STATE_SET_PIN_CODE;
};

state_func_t* const stateTable[NUM_STATES] = {
    runStateInitial, runStateTryPincode, runStateSetPincode
};

state_t runState(state_t currentState, unsigned char stateInput) {
    lockInput = ' ';
    return stateTable[currentState](stateInput);
};

void lockInit (void) {
    currentState = STATE_INITIAL;
    lockInput = ' ';
}

void setlockInput(unsigned char input){
    lockInput = input;
}

void lockRun(){
    currentState = runState(currentState, lockInput);
}