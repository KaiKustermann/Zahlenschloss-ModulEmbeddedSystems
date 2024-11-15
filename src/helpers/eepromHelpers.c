#include <avr/eeprom.h>

#include "eepromHelpers.h"

// function to reset EEPROM to initial values
void eepromReset() {
    // write the default values to EEPROM
    for (uint16_t address = 0; address < EEPROM_SIZE; ++address) {
        eeprom_write_byte((uint8_t*)address, EEPROM_DEFAULT_VALUE_PER_BYTE);
    }
}