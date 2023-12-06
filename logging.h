#include "enums.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void loggerInit();
    void logMessage(char* message, LogLevel level);

#ifdef __cplusplus
}
#endif