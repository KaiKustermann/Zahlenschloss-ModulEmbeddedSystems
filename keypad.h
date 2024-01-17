#ifdef __cplusplus
extern "C"
{
#endif

#ifndef KEYPAD_H
#define KEYPAD_H
    typedef void on_key_changed_function_t(unsigned char pressedButton);
    void keypadInit();
    uint8_t findPressedKey();
    uint8_t hasKeyChanged();
    uint8_t hasKeyBeenReleased();
    uint8_t getPressedKey();
    uint32_t getTimeSinceKeyPressInit();
    void keypadRun();
#endif 

#ifdef __cplusplus
}
#endif
