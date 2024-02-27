#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "logging.h"

uint8_t loggingInitialized = 0;

// initializes logging via usart
void loggingInit(){
    // initialize usart for logging, if it is not initialized already
    if(isUsartInitialized() == 0){
        usartInit();
    }
    loggingInitialized = 1;
}

// helper function to log the log level
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

// logs a char* message
void logMessage(char* message, LogLevel level){
    if(loggingInitialized){
        logLogLevel(level);
        usartPutString(message);
        usartPutString("; \r\n");
    }
}

// logs a uint8_t message
void logMessageInt(uint8_t message, LogLevel level){
    if(loggingInitialized){
        char str[4];
        sprintf(str, "%u", message);
        logMessage(str, level);
    }
}

// logs a unsigned char message
void logMessageChar(unsigned char message, LogLevel level){
    if(loggingInitialized){
        logLogLevel(level);
        usartPutChar(message);
        usartPutString("; ");
    }
}

// logs uint32_t message 
void logMessageUInt32(uint32_t message, LogLevel level) {
    if(loggingInitialized){
        char str[12];
        sprintf(str, "%lu", (unsigned long)message);
        logMessage(str, level);
    }
}