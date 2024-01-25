#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#include "keypad.h"
#include "logging.h"
#include "uart.h"
#include "timerHelpers.h"

#define DDR_KEYPAD_ROWS DDRD
#define DDR_KEYPAD_COLS DDRB
#define PORT_KEYPAD_ROWS PORTD
#define PORT_KEYPAD_COLS PORTB
#define PIN_KEYPAD_ROWS PIND
#define PIN_KEYPAD_COLS PINB
#define PCIE_KEYPAD PCIE0
#define PCMSK_KEYPAD PCMSK0
#define PCINT_KEYPAD PCINT0_vect
// mapping to pins of ports
#define ROW_0 4
#define ROW_1 5
#define ROW_2 6
#define ROW_3 7
#define COL_0 0
#define COL_1 1
#define COL_2 2
#define COL_3 3

uint8_t keypad[4][4] = {{'1','2','3','A'},
                        {'4','5','6','B'},
                        {'7','8','9','C'},
                        {'*','0','#','D'}};

uint8_t keyChanged = 0; //flag that is a non zero value if a key has changed and 0 if it has not

// 0 when no key pressed
uint8_t currentPressedKey = 0;

on_key_changed_function_t* keyPressHandler;

// using column scanning technique (columns as input to the microcontroller); 
// detailed: going through the rows (outputs to microcontroller) and applying ground (0) to each row. Then reading the columns (inputs to microcontroller). If the column is 0, the button in this row for this column was pressed.
void keypadInit(){
    // set rows as output (set to 1) using the data direction registry
    DDR_KEYPAD_ROWS |= (1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3);
    // set columns as input (set to 0) using the data direction registry; detailed: 00000001 | 00000010 | 00000100| 00001000 -> 00001111 -> 11110000 -> with bitwise and the pins with 1 stay 1 (e.g. before: DDR_KEYPAD = 10101010; mask: 11110000; result: 10100000)
    DDR_KEYPAD_COLS &= ~((1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3));
    // set internal pullup for columns (set to 1) by setting the right ports to 1
    PORT_KEYPAD_COLS |= (1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3);
    // set rows as low (set to 0) by setting the right ports to 0 
    PORT_KEYPAD_ROWS  &= ~((1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3));
    // enable pin change interrupt for port of keypad
    PCICR |= (1 << PCIE_KEYPAD);
    // control which pins you want to get the interrupt from (all columns)
    PCMSK_KEYPAD |= (1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3);
}   

uint8_t findPressedKey(){
    // disable pin change interrupt for pins
    PCMSK_KEYPAD &= ~((1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3));
    const uint8_t rows[4] = {ROW_0, ROW_1, ROW_2, ROW_3};
    const uint8_t cols[4] = {COL_0, COL_1, COL_2, COL_3};
    for (uint8_t row=0; row<4; row++){
        // set all row high (1)
        PORT_KEYPAD_ROWS  |= (1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3);
        // set current row to low (0)
        PORT_KEYPAD_ROWS &=~(1<<rows[row]);
        for(uint8_t col=0; col<4; col++){
            // check if column is low (0), therefore pressed
            if((PIN_KEYPAD_COLS & (1<<cols[col])) == 0){
                // set rows as low (set to 0) by setting the right ports to 0 
                PORT_KEYPAD_ROWS  &= ~((1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3));
                // enable pin change interrupt for pins
                PCMSK_KEYPAD |= (1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3);
                return keypad[row][col];
            }
        }
    }
    // set rows as low (set to 0) by setting the right ports to 0 
    PORT_KEYPAD_ROWS  &= ~((1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3));
    // enable pin change interrupt for pins
    PCMSK_KEYPAD |= (1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3);
    return 0;
}

// returns a non zero value if the key has been changed (pressed or released)
uint8_t hasKeyChanged(){
    return keyChanged;
}

void setKeyPressHandler(on_key_changed_function_t* keyPressHandlerFunction){
    keyPressHandler = keyPressHandlerFunction;
}

void keypadRun(){
    // handle case where key has changed (has been pressed or released) 
    if(hasKeyChanged() != 0){
        uint8_t pressedKey = findPressedKey();
        // handle case where key has been pressed
        if(pressedKey != 0){
            initTimer(); 
            currentPressedKey = pressedKey;
            struct keyEvent keyPressEvent;
            keyPressEvent.pressEventType = KEY_PRESS_START;
            keyPressEvent.pressedKey = (unsigned char) currentPressedKey;
            keyPressEvent.pressDuration = getMillis();
            keyPressHandler(keyPressEvent);
        } 
        // handle case where key has been released
        else {
            struct keyEvent keyPressEvent;
            keyPressEvent.pressEventType = KEY_PRESS_END;
            keyPressEvent.pressedKey = (unsigned char) currentPressedKey;
            keyPressEvent.pressDuration = getMillis();
            keyPressHandler(keyPressEvent);
            // set current pressed key to default value (0 when no key pressed)
            currentPressedKey = 0;
            resetTimer();
        }
        // reset keyChanged variable
        keyChanged = 0;
    } 
    // handle case where a key is currently pressed
    else if (currentPressedKey != 0){
        struct keyEvent keyPressEvent;
        keyPressEvent.pressEventType = KEY_HOLD;
        keyPressEvent.pressedKey = (unsigned char) currentPressedKey;
        keyPressEvent.pressDuration = getMillis();
        keyPressHandler(keyPressEvent);
    }
    // handle case where no key is currently pressed, released or hold
    else {
        struct keyEvent keyPressEvent;
        keyPressEvent.pressEventType = KEY_NONE;
        keyPressHandler(keyPressEvent);
    }
}

// interrupt service routine for pins of columns
ISR(PCINT_KEYPAD){
    keyChanged = 1;
}