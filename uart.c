#include <avr/io.h>

#define BAUD 9600UL // baudrate

#define UBRR_VAL ((F_CPU)/(BAUD*16)-1) // calculate ubrr value
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1))) // real baudrate calculated from ubrr value
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // calculates error in promille, where 1000 = no error

#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error baudrate error higher then one percent and therefore too high!
#endif

static uint8_t uartInitialized = 0;

void usartInit() {
    unsigned int ubrr = UBRR_VAL;
    /* set baud rate 
    setting the 8 high significance bits (first 8) of the 16 bit value of ubrr to UBRR0H (high register) and the 8 low significance bits (last 8) to UBRR0L (low register).
    by using the typecast to unsigned char (8 bit value) we make sure that only the low significance bits (last 8) bits are taken */
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    /* enable transmitter */
    UCSR0B = (1<<TXEN0);
    /* set frame format: 8 bit data, 1 stop bit */
    UCSR0C = (0<<USBS0)|(3<<UCSZ00);
    uartInitialized = 1;
}

uint8_t isUsartInitialized(){
    return uartInitialized;
}

/* send a char via usart */
int usartPutChar(unsigned char c)
{
    /* wait for empty transmit buffer
    checking the USART Data Register Empty (UDRE0) of register UCSR0A
    through the bitmask 00010000 and the bitwise "and" operation (returns only 1 if both bits are 1), it is checked if the UDRE0 bit currently is 1
    if the UDRE0 bit is set to 1 the buffer is empty, so while it evaluates to 0 (false) the loop keeps spinning -> waits until set to 1, therefore empty
    */
    while (!(UCSR0A & (1<<UDRE0)))
    {
    }                             
    /* put data into buffer, sends the data */
    UDR0 = c;
    return 0;
}

/* send a string via usart */
void usartPutString (char *s)
{
    while (*s)
    { 
        usartPutChar(*s);
        s++;
    }
}