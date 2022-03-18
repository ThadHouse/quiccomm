#pragma once

#include "CommTypes.h"
#include <stdint.h>

typedef void (COMMLIB_API * HPTimer_Callback)(void* Context);

typedef struct HPTimer HPTimer;

#ifdef __cplusplus
extern "C" {
#endif

CommLibStatus HPTimer_Initialize(HPTimer_Callback Callback, void* Context, uint32_t PeriodMs, HPTimer** Thread);

void HPTimer_Free(HPTimer* Thread);

uint64_t HPTimer_GetTimeUs(void);

#ifdef __cplusplus
}
#endif
