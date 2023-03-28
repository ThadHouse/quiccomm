#include "dscomm.h"
#include <catomic.h>
#include <c11threads.h>
#include "HPTimer.h"
#include "CQuicConnection.h"
#include "msquic.h"

struct DSComm {
    thrd_t CommThread;
};

static int DSThread(void* Context) {
    (void)Context;
    QuicRegistration* Registration;
    CommLibStatus Status = QC_GetRegistration("Netcomm", TRUE, &Registration);
    (void)Status;
    return 0;
}

CommLibStatus COMMLIB_API DSComm_Initialize(DSComm** Comm) {
    CommLibStatus Status;
    DSComm* NewComm = malloc(sizeof(DSComm));
    if (!NewComm) {
        return QUIC_STATUS_OUT_OF_MEMORY;
    }
    memset(NewComm, 0, sizeof(*NewComm));

    Status = thrd_create(&NewComm->CommThread, DSThread, NewComm);
    if (COMMLIB_FAILED(Status)) {
        goto Exit;
    }

    Status = QUIC_STATUS_SUCCESS;
    *Comm = NewComm;
    NewComm = NULL;

Exit:
    if (NewComm) {
        if (NewComm->CommThread) {
            thrd_join(NewComm->CommThread, NULL);
        }
        free(NewComm);
    }
    return Status;
}

void COMMLIB_API DSComm_Free(DSComm* Comm) {
    thrd_join(Comm->CommThread, NULL);
    free(Comm);
}
`
