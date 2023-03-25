#pragma once

#ifdef __STDC_NO_THREADS__
#ifdef _WIN32
#include "winshimthread.h"
#else
#error No atomic implementation on non win32
#endif
#else
#include <threads.h>
#endif