#include <avr/io.h>
#include <util/delay.h>

#define DDR_KEYPAD DDRD
#define PORT_KEYPAD PORTD
#define PIN_KEYPAD PIND
#define ROW_0 0
#define ROW_1 1
#define ROW_2 2
#define ROW_3 3
#define COL_0 4
#define COL_1 5
#define COL_2 6
#define COL_3 7

uint8_t keypad[4][4] = {{'1','2','3','A'},
                        {'4','5','6','B'},
                        {'7','8','9','C'},
                        {'*','0','#','D'}};

// using column scanning technique (columns as input to the microcontroller); 
// detailed: going through the rows (outputs to microcontroller) and applying ground (0) to each row. Then reading the columns (inputs to microcontroller). If the column is 0, the button in this row for this column was pressed.
void keypadInit(){
    // set rows as output (set to 1) using the data direction registry
    DDR_KEYPAD |= (1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3);
    // set columns as input (set to 0) using the data direction registry; detailed: 00000001 | 00000010 | 00000100| 00001000 -> 00001111 -> 11110000 -> with bitwise and the pins with 1 stay 1 (e.g. before: DDR_KEYPAD = 10101010; mask: 11110000; result: 10100000)
    DDR_KEYPAD &= ~((1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3));
    // set internal pullup for columns (set to 1) by setting the right ports to 1
    PORT_KEYPAD |= (1 << COL_0) | (1 << COL_1) | (1 << COL_2) | (1 << COL_3);
    // set rows as high (set to 1) by setting the right ports to 1 
    PORT_KEYPAD  |= (1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3);
}   

uint8_t findPressedKey(){
    const uint8_t rows[4] = {ROW_0, ROW_1, ROW_2, ROW_3};
    const uint8_t cols[4] = {COL_0, COL_1, COL_2, COL_3};
    for (uint8_t row=0; row<4; row++){
        // set all row high (1)
        PORT_KEYPAD  |= (1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3);
        // set current row to low (0)
        PORT_KEYPAD &=~(1<<rows[row]);
        for(uint8_t col=0; col<4; col++){
            // check if column is low (0), therefore pressed
            if((PIN_KEYPAD & (1<<cols[col])) == 0){
                return keypad[row][col];
            }
        }
    }
    return 0;
}