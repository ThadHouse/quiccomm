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