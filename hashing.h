#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HASHING_H
#define HASHING_H
    void generateSalt(char *salt, size_t saltSize);
    void hashPincode(char *pincode, char *hashedPincode, char* salt);
#endif 

#ifdef __cplusplus
}
#endif
