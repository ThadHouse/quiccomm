#pragma once

#include "CommTypes.h"
#include <stdint.h>

typedef void(COMMLIB_API* HPTimer_Callback)(void* Context);

typedef struct HPTimer HPTimer;

#ifdef __cplusplus
extern "C" {
#endif

CommLibStatus COMMLIB_API HPTimer_Initialize(HPTimer_Callback Callback,
                                             void* Context, uint32_t PeriodMs,
                                             HPTimer** Thread);

void COMMLIB_API HPTimer_Free(HPTimer* Thread);

uint64_t COMMLIB_API HPTimer_GetTimeUs(void);

void COMMLIB_API HPTimer_SleepMs(uint32_t TimeMs);

#ifdef __cplusplus
}
#endif
