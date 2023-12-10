#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"
#include "enums.h"

void loggerInit(){
    if(isUartInitialized() == 0){
        usartInit();
    }
}

void logMessage(char* message, LogLevel level){
    char* prefix;
    switch (level) {
        case INFO:
            prefix = "INFO: ";
            break;
        case WARNING:
            prefix = "WARNING: ";
            break;
        case ERROR:
            prefix = "ERROR: ";
            break;
        default:
            prefix = "UNKNOWN: ";
            break;
    }

    char completeMessage[strlen(prefix) + strlen(message) + 1]; 

    strcpy(completeMessage, prefix);

    strcat(completeMessage, message);

    uartPutString(completeMessage);
}
