#ifdef __cplusplus
extern "C"
{
#endif

#ifndef KEYPAD_H
#define KEYPAD_H
    typedef enum {
        // KEY_NONE when no key is pressed
        KEY_NONE,
        KEY_PRESS_START,
        KEY_PRESS_END,
        KEY_HOLD
    } keyEventType;
    struct keyEvent {
        keyEventType pressEventType;
        unsigned char pressedKey;
        uint32_t pressDuration;
    };
    typedef void on_key_changed_function_t(struct keyEvent keyPressEvent);
    void keypadInit();
    void keypadRun();
    void setKeyPressHandler(on_key_changed_function_t* keyPressHandlerFunction);
#endif 

#ifdef __cplusplus
}
#endif
