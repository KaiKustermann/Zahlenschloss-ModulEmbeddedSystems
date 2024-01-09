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
            usartPutString("INFO: ");
            break;
        case WARNING:
            usartPutString("WARNING: ");
            break;
        case ERROR:
            usartPutString("ERROR: ");
            break;
        default:
            usartPutString("INFO: ");
            break;
    }
}

void logMessage(char* message, LogLevel level){
    logLogLevel(level);
    usartPutString(message);
    usartPutString("; ");
}

void logMessageInt(uint8_t message, LogLevel level){
    char str[4];
    sprintf(str, "%u", message);
    logMessage(str, level);
}

void logMessageChar(unsigned char message, LogLevel level){
    logLogLevel(level);
    usartPutChar(message);
    usartPutString("; ");
}