#pragma once

#ifdef __STDC_NO_ATOMICS__
#ifdef _WIN32
#include "winshimatomic.h"
#else
#error No atomic implementation on non win32
#endif
#else
#include <stdatomic.h>
#endif

typedef volatile int atomic_bool;

#define atomic_init(obj, value) \
    do {\
      *(obj) = value; \
    } while (0)

#define atomic_store(obj, value) \
    do {\
      *(obj) = value; \
      MemoryBarrier(); \
    } while (0)

#define atomic_load(obj) \
    (MemoryBarrier(), *(obj))
