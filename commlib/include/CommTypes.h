#pragma once

#include <stdint.h>

typedef int32_t CommLibStatus;

#define COMMLIB_FAILED(X)                  ((int)(X) > 0)

#ifdef _WIN32
#define COMMLIB_API __cdecl
#else
#define COMMLIB_API
#endif

typedef uint32_t CommLibBoolean;

#define COMMLIB_TRUE 1
#define COMMLIB_FALSE 0
