#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "logging.h"

uint8_t loggingInitialized = 0;

void loggingInit(){
    if(isUsartInitialized() == 0){
        usartInit();
    }
    loggingInitialized = 1;
}

void logLogLevel(LogLevel level){
    if(loggingInitialized){
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
}

void logMessage(char* message, LogLevel level){
    if(loggingInitialized){
        logLogLevel(level);
        usartPutString(message);
        usartPutString("; \r\n");
    }
}

void logMessageInt(uint8_t message, LogLevel level){
    if(loggingInitialized){
        char str[4];
        sprintf(str, "%u", message);
        logMessage(str, level);
    }
}

void logMessageChar(unsigned char message, LogLevel level){
    if(loggingInitialized){
        logLogLevel(level);
        usartPutChar(message);
        usartPutString("; ");
    }
}

void logMessageUInt32(uint32_t message, LogLevel level) {
    if(loggingInitialized){
        char str[12];
        sprintf(str, "%lu", (unsigned long)message);
        logMessage(str, level);
    }
}