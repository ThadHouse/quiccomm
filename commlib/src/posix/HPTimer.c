#include "HPTimer.h"

CommLibStatus COMMLIB_API HPTimer_Initialize(HPTimer_Callback Callback,
                                             void* Context, uint32_t PeriodMs,
                                             HPTimer** Thread) {
                                                 (void)Callback;
                                                 (void)Context;
                                                 (void)PeriodMs;
                                                 (void)Thread;

                                                 return 0;
                                             }

void COMMLIB_API HPTimer_Free(HPTimer* Thread) {
    (void)Thread;
}

uint64_t COMMLIB_API HPTimer_GetTimeUs(void) {
    return 0;
}

void COMMLIB_API HPTimer_SleepMs(uint64_t TimeMs) {
    
}