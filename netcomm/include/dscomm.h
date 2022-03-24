#pragma once

#include "CommTypes.h"

typedef struct DSComm DSComm;

#ifdef __cplusplus
extern "C" {
#endif

CommLibStatus COMMLIB_API DSComm_Initialize(DSComm** Comm);

void COMMLIB_API DSComm_Free(DSComm* Comm);

#ifdef __cplusplus
}
#endif
