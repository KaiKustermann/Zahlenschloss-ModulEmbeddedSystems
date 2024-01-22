#include "program.h"
#include "eepromHelpers.h"

int main()
{
    // eepromReset();
    setup();
    while (1)
    {
        loop();
    }
}