#pragma once

#include "CommTypes.h"
#include <stdint.h>

typedef void (COMMLIB_API * HPThread_Callback)(void* Context);

typedef struct HPThread HPThread;

#ifdef __cplusplus
extern "C" {
#endif

CommLibStatus HPThread_Initialize(HPThread_Callback Callback, void* Context, uint32_t PeriodMs, HPThread** Thread);

void HPThread_Free(HPThread* Thread);

uint64_t HPThread_GetTimeUs(void);

#ifdef __cplusplus
}
#endif
