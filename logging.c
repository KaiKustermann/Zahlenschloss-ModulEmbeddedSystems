#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "enums.h"

void loggerInit(){
    if(isUsartInitialized() == 0){
        usartInit();
    }
}

void logLogLevel(LogLevel level){
    switch (level) {
        case INFO:
            uartPutString("INFO: ");
            break;
        case WARNING:
            uartPutString("WARNING: ");
            break;
        case ERROR:
            uartPutString("ERROR: ");
            break;
        default:
            uartPutString("INFO: ");
            break;
    }
}

void logMessage(char* message, LogLevel level){
    logLogLevel(level);
    uartPutString(message);
    uartPutString("; ");
}

void logMessageInt(uint8_t message, LogLevel level){
    logLogLevel(level);
    char messageAsChar = '0' + message;
    usartPutChar(messageAsChar);
    uartPutString("; ");
}

void logMessageChar(unsigned char message, LogLevel level){
    logLogLevel(level);
    usartPutChar(message);
    uartPutString("; ");
}