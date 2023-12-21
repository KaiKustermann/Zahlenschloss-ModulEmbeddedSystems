#include <string.h>

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