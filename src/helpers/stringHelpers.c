#include <string.h>
#include <avr/io.h>

// sets the last character of the given string to the null terminator \0
void strDeleteLastCharacter(char* string){
    if (string != NULL){
        size_t len = strlen(string);
        if (len > 0) {
            string[len - 1] = '\0';
        } 
    }
}

// sets the first character of the given string to the null terminator \0
void strClear(char* string){
    if(string != NULL){
        string[0] = '\0';
    }
}

// compares two strings, where the execution time is independent of the length of s2 (therefore s1 should be user input)
// returns 0 if both strings contain the same value, a non zero value if the strings are different
uint8_t strCmpConstantTime(const char *s1, const char *s2) {
    uint8_t result = 0;
    // volatile keyword, that the cpu loads it always into the register from memory and does not keep it -> always same time behavior 
    volatile size_t index_s1 = 0;
    volatile size_t index_s2 = 0;
    volatile size_t index_s2_decoy = 0;

    // NULL check to ensure that s1 and s2 contain proper values
    if (s1 == NULL || s2 == NULL)
        return 1;

    while (1) {
        // xor comparison of two characters, which returns 1 if the chars contain different values, otherwise returns 0
        result |= s1[index_s1]^s2[index_s2];
        // break if end of s1 is reached, reveals the length of s1 by time
        // we need to do the comparison before this line to avoid a substring match
        if (s1[index_s1] == '\0')
            break;
        index_s1++;

        if (s2[index_s2] != '\0')
            index_s2++;
        // doing the same operation with same time behavior if null terminator reached, to not reveal the string length
        if (s2[index_s2] == '\0')
            index_s2_decoy++;
    }
    return result;
}