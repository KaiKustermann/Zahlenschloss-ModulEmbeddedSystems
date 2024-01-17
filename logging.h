#include "enums.h"
#include <avr/io.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void loggerInit();
    void logMessage(char* message, LogLevel level);
    void logMessageInt(uint8_t message, LogLevel level);
    void logMessageChar(unsigned char message, LogLevel level);
    void logMessageUInt32(uint32_t message, LogLevel level);

#ifdef __cplusplus
}
#endif