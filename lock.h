#include "keypad.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LOCK_H
#define LOCK_H
    typedef enum { STATE_INITIAL, STATE_TRY_PIN_CODE, STATE_SET_PIN_CODE_INITIAL, STATE_SET_PIN_CODE_SUBSTATE_ENTER_CURRENT, STATE_SET_PIN_CODE_SUBSTATE_ENTER_NEW, STATE_OPEN, STATE_RESET, NUM_STATES } state_t;
    typedef struct lock lock_t;
    typedef state_t state_func_t();
    void lockInit (void);
    void setLockInput(struct keyEvent keyPress);
    void lockRun();
#endif 

#ifdef __cplusplus
}
#endif