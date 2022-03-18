#pragma once

#include "CommTypes.h"
#include <stdint.h>

typedef void(COMMLIB_API* CPThread_Main)(void* Context);

typedef struct CPThread CPThread;

#ifdef __cplusplus
extern "C" {
#endif

CommLibStatus COMMLIB_API CPThread_Initialize(CPThread_Main ThreadMain, void* Context, CPThread** Thread);

void COMMLIB_API CPThread_Free(CPThread* Thread);

#ifdef __cplusplus
}
#endif