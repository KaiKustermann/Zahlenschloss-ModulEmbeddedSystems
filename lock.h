#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LOCK_H
#define LOCK_H
    typedef enum { STATE_INITIAL, STATE_TRY_PIN_CODE, STATE_SET_PIN_CODE, NUM_STATES } state_t;
    typedef struct lock lock_t;
    typedef state_t state_func_t(unsigned char data);
    void lockInit (void);
    void setlockInput(unsigned char input);
    void lockRun();
#endif 

#ifdef __cplusplus
}
#endif