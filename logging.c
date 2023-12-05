#include <avr/io.h>
#include <stdio.h>
#include <string.h>

#include "uart.h"

// TODO
// void loggerInit(){
//     if(usartInitialized == 0){
//         usartInit();
//     }
// }

// TODO loglevel
void logMessage(char* message){
    const char* prefix;
    // switch (level) {
    //     case INFO:
    //         prefix = "INFO";
    //         break;
    //     case WARNING:
    //         prefix = "WARNING";
    //         break;
    //     case ERROR:
    //         prefix = "ERROR";
    //         break;
    //     default:
    //         prefix = "UNKNOWN";
    //         break;
    // }
    
    // char dest[60];
    // strcpy(dest, prefix);

    // // adjust the remaining space in dest for the message
    // size_t remainingSpace = sizeof(dest) - strlen(dest) - 1;  // -1 for the null terminator

    // // concatenate the message, ensuring not to exceed the remaining space
    // strncat(dest, message, remainingSpace);

    // use uartPutString to output the result
    uartPutString(message);
}
