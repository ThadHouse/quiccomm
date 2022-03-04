#include "QuicApi.h"

namespace qapi {

const QUIC_API_TABLE* MsQuic;

bool InitializeMsQuic() {
    QUIC_STATUS Status = MsQuicOpen2(&::qapi::MsQuic);
    return QUIC_SUCCEEDED(Status);
}

void FreeMsQuic() {
    if (MsQuic) {
        MsQuicClose(MsQuic);
    }
}

}
