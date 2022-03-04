#include "QuicApi.h"

bool InitializeDS() {
    return ::qapi::InitializeMsQuic();
}

void FreeDS() {
    ::qapi::FreeMsQuic();
}
