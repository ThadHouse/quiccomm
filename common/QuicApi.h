#pragma once

#define QUIC_API_ENABLE_INSECURE_FEATURES
#define QUIC_API_ENABLE_PREVIEW_FEATURES
#include "msquic.h"

namespace qapi {

extern const QUIC_API_TABLE* MsQuic;

bool InitializeMsQuic();

void FreeMsQuic();

}
