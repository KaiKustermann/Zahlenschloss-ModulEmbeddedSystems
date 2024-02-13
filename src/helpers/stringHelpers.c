#include <string.h>
#include <avr/io.h>

void strDeleteLastCharacter(char* string){
    if (string != NULL){
        size_t len = strlen(string);
        if (len > 0) {
            string[len - 1] = '\0';
        } 
    }
}

void strClear(char* string){
    if(string != NULL){
        string[0] = '\0';
    }
}

// compares to strings, where the execution time is independent of the length of s2 (therefore s1 should be user input)
// returns 0 if both strings contain the same value, a non zero value if the strings are different
uint8_t strCmpConstantTime(const char *s1, const char *s2) {
    // volatile keyword, that the cpu loads it always into the register from memory and does not keep it -> always same time behavior 
    uint8_t result = 0;
    volatile size_t index_s1 = 0;
    volatile size_t index_s2 = 0;
    volatile size_t index_s2_decoy = 0;

    if (s1 == NULL || s2 == NULL)
        return 1;

    while (1) {
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