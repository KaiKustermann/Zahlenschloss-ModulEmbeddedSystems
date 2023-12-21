#ifdef __cplusplus
extern "C"
{
#endif

#ifndef KEYPAD_H
#define KEYPAD_H
    typedef void on_key_changed_function_t(unsigned char pressedButton);
    void keypadInit();
    uint8_t findPressedKey();
    void setOnKeyChangedHandler(on_key_changed_function_t* handler);
    uint8_t hasKeyChanged();
    void handleKeyChange(on_key_changed_function_t* handler);
#endif 

#ifdef __cplusplus
}
#endif
