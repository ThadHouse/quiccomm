#include "QuicConnection.h"
#ifdef _WIN32
//
// The conformant preprocessor along with the newest SDK throws this warning for
// a macro in C mode. As users might run into this exact bug, exclude this
// warning here. This is not an MsQuic bug but a Windows SDK bug.
//
#pragma warning(disable:5105)
#endif
#include "msquic.h"
#include <stdlib.h>

struct QuicRegistration {
    const QUIC_API_TABLE* QuicApi;
    HQUIC Registration;
};

struct QuicListener {
    const QuicRegistration* Registration;
    HQUIC Listener;
};

QuicConnStatus QC_GetRegistration(const char* Name, QuicConnBoolean UseSingleThread, QuicRegistration** Registration) {
    QUIC_STATUS Status;
    QuicRegistration* NewRegistration = malloc(sizeof(QuicRegistration));

    if (!NewRegistration) {
        return QUIC_STATUS_OUT_OF_MEMORY;
    }
    memset(NewRegistration, 0, sizeof(*NewRegistration));

    Status = MsQuicOpen2(&NewRegistration->QuicApi);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    QUIC_REGISTRATION_CONFIG RegConfig = {
        .AppName = Name,
        .ExecutionProfile = QUIC_EXECUTION_PROFILE_LOW_LATENCY
    };
    if (UseSingleThread) {
        RegConfig.ExecutionProfile = QUIC_EXECUTION_PROFILE_TYPE_SCAVENGER;
    }

    Status = NewRegistration->QuicApi->RegistrationOpen(&RegConfig, &NewRegistration->Registration);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = QUIC_STATUS_SUCCESS;
    *Registration = NewRegistration;
    NewRegistration = NULL;

Exit:
    if (NewRegistration) {
        if (NewRegistration->Registration) {
            NewRegistration->QuicApi->RegistrationClose(NewRegistration->Registration);
        }
        if (NewRegistration->QuicApi) {
            MsQuicClose(NewRegistration->QuicApi);
        }
    }
    return Status;
}

void QC_FreeRegistration(QuicRegistration* Registration) {
    if (Registration) {
        Registration->QuicApi->RegistrationClose(Registration->Registration);
        MsQuicClose(Registration->QuicApi);
        free(Registration);
    }
}

QuicConnStatus QC_CreateListener(const QuicRegistration* Registration, uint16_t Port, uint8_t* PfxBuffer, uint32_t PfxSize, const char* PfxPassword, uint32_t NumStreams, QuicListenerNewConnectionCallback Callback, void* Context, QuicListener** Listener) {
    QUIC_STATUS Status;
    QuicListener* NewListener = malloc(sizeof(QuicListener));
    if (!NewListener) {
        return QUIC_STATUS_OUT_OF_MEMORY;
    }
    memset(NewListener, 0, sizeof(*NewListener));
    NewListener->Registration = Registration;

    QUIC_CREDENTIAL_CONFIG CredConfig;
    memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_PKCS12;
    QUIC_CERTIFICATE_PKCS12 Pkcs12;
    Pkcs12.Asn1Blob = PfxBuffer;
    Pkcs12.Asn1BlobLength = PfxSize;
    Pkcs12.PrivateKeyPassword = PfxPassword;

    CredConfig.CertificatePkcs12 = &Pkcs12;

    Status = Registration->QuicApi->ConfigurationOpen(Registration->Registration, )

    *Listener = NewListener;
    NewListener = NULL;

Exit:
    if (NewListener) {
        if (NewListener->Listener) {
            NewListener->Registration->QuicApi->ListenerClose(NewListener->Listener);
        }
    }

    return Status;
}

void QC_StartListener(QuicListener* Listener);
void QC_StopListener(QuicListener* Listener);

void QC_FreeListener(QuicListener* Listener) {
    if (Listener) {
        Listener->Registration->QuicApi->ListenerClose(Listener->Listener);
    }
}
