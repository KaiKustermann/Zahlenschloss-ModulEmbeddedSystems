#include <avr/io.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LOGGING_H
#define LOGGING_H
    typedef enum {
            INFO,
            WARNING,
            ERROR
        } LogLevel;
    void loggingInit();
    void logMessage(char* message, LogLevel level);
    void logMessageInt(uint8_t message, LogLevel level);
    void logMessageChar(unsigned char message, LogLevel level);
    void logMessageUInt32(uint32_t message, LogLevel level);
#endif 

#ifdef __cplusplus
}
#endif