#include "dscomm.h"
#include "CPThread.h"
#include "HPTimer.h"
#include "CQuicConnection.h"
#include "msquic.h"

struct DSComm {
    CPThread* CommThread;

};

static void DSThread(void* Context) {
    (void)Context;
    QuicRegistration* Registration;
    CommLibStatus Status = QC_GetRegistration("Netcomm", TRUE, &Registration);
    (void)Status;
}

CommLibStatus COMMLIB_API DSComm_Initialize(DSComm** Comm) {
    CommLibStatus Status;
    DSComm* NewComm = malloc(sizeof(DSComm));
    if (!NewComm) {
        return QUIC_STATUS_OUT_OF_MEMORY;
    }
    memset(NewComm, 0, sizeof(*NewComm));

    Status = CPThread_Initialize(DSThread, NewComm, &NewComm->CommThread);
    if (COMMLIB_FAILED(Status)) {
        goto Exit;
    }

    Status = QUIC_STATUS_SUCCESS;
    *Comm = NewComm;
    NewComm = NULL;

Exit:
    if (NewComm) {
        if (NewComm->CommThread) {
            CPThread_Free(NewComm->CommThread);
        }
        free(NewComm);
    }
    return Status;
}

void COMMLIB_API DSComm_Free(DSComm* Comm) {
    CPThread_Free(Comm->CommThread);
    free(Comm);
}
