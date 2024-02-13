#include "./lock/program.h"
#include "./helpers/eepromHelpers.h"

int main()
{
    // eepromReset();
    setup();
    while (1)
    {
        loop();
    }
}