#ifdef __cplusplus
extern "C"
{
#endif

#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H
    void strDeleteLastCharacter(char* string);
    void strClear(char* string);
    uint8_t strCmpConstantTime(const char *s1, const char *s2);
#endif 

#ifdef __cplusplus
}
#endif