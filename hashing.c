#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "hashing.h"

// djb2 hashing function
uint32_t hashDJB2(const char *str) {
    uint32_t hash = 5381;
    uint8_t c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // equals hash * 33 + c
    }

    return hash;
}

// generate a random salt with chars ranging from uppercase A to Z with given salt size 
void generateSalt(char *salt, size_t saltSize) {
    for (size_t i = 0; i < saltSize - 1; ++i) {
        // the result of rand() % 26 is added to the ASCII value of the uppercase letter A
        // through using modulo a max value of 25 is returned by rand() % 26
        // so random uppercase letters from A to Z are returned
        salt[i] = 'A' + rand() % 26;
    }
    salt[saltSize - 1] = '\0'; 
}

// hash the password with salt
void hashPincode(char *pincode, char *hashedPincode, char* salt) {
    // the format specifier %08lx pads the returned string with leading zeros, if the string is less than 8 hexadecimal values long
    snprintf(hashedPincode, sizeof(hashedPincode), "%08lx", hashDJB2(pincode) ^ hashDJB2(salt));
}