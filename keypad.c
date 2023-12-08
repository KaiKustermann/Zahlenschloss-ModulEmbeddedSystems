#include <avr/io.h>
#include <util/delay.h>

#define DDR_KEYPAD DDRD
#define PORT_KEYPAD PORTD
#define ROW_0 0
#define ROW_1 1
#define ROW_2 2
#define ROW_3 3
#define COL_0 0
#define COL_1 1
#define COL_2 2
#define COL_3 3

keypadInit(){
    // set columns as output (set to 1) using the data direction registry
    DDR_KEYPAD |= (1U << COL_0) | (1U << COL_1) | (1U << COL_2) | (1U << COL_3);
    // set rows as input (set to 0) using the data direction registry; detailed: 00000001 | 00000010 | 00000100| 00001000 -> 00001111 -> 11110000 -> with bitwise and the pins with 1 stay 1 (e.g. before: DDR_KEYPAD = 10101010; mask: 11110000; result: 10100000)
    DDR_KEYPAD &= ~((1 << ROW_0) | (1 << ROW_1) | (1 << ROW_2) | (1 << ROW_3));
    // set internal pullup for rows (set to 1) by setting the right ports (output) to 1
    PORT_KEYPAD |= (1U << ROW_0) | (1U << ROW_1) | (1U << ROW_2) | (1U << ROW_3);
    // set columns as high (set to 1) by setting the right ports (output) to 1
    PORT_KEYPAD |= (1U << COL_0) | (1U << COL_1) | (1U << COL_2) | (1U << COL_3);
}   

findPressedKey(){

}