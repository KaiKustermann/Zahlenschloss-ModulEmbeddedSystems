#ifdef __cplusplus
extern "C"
{
#endif
    void usartInit();
    uint8_t isUartInitialized();
    void usartPutChar(unsigned char c);
    void uartPutString(char *s);

#ifdef __cplusplus
}
#endif