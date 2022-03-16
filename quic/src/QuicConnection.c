#include "QuicConnection.h"
#ifdef _WIN32
//
// The conformant preprocessor along with the newest SDK throws this warning for
// a macro in C mode. As users might run into this exact bug, exclude this
// warning here. This is not an MsQuic bug but a Windows SDK bug.
//
#pragma warning(disable:5105)
#pragma warning(disable:4200)  // nonstandard extension used: zero-sized array in struct/union
#endif
#include "msquic.h"
#include <stdlib.h>

#define BYTESWAPSHORT(x) ((uint16_t)((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8)))

struct QuicRegistration {
    const QUIC_API_TABLE* QuicApi;
    HQUIC Registration;
};

struct QuicListener {
    const QuicRegistration* Registration;
    HQUIC Listener;
    HQUIC Configuration;
    QuicListenerCallbacks Callbacks;
    uint32_t NumStreams;
    uint16_t Port;
    uint16_t AlpnLength;
    uint8_t Alpn[0];
};

struct QuicConnection {
    const QuicRegistration* Registration;
    HQUIC Connection;
    QuicConnectionCallbacks Callbacks;
    uint32_t NumStreams;
    HQUIC Streams[0];
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

static QUIC_STATUS QUIC_API ListenerCallback(HQUIC Listener, void* Context, QUIC_LISTENER_EVENT* Event) {
    (void)Listener;
    QuicListener* QListener = (QuicListener*)Context;

    if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION) {
        QuicConnection* NewConnection = malloc(sizeof(QuicConnection) + QListener->NumStreams);
        if (!NewConnection) {
            return QUIC_STATUS_OUT_OF_MEMORY;
        }
        memset(NewConnection, 0, sizeof(*NewConnection) + QListener->NumStreams);
        NewConnection->NumStreams = QListener->NumStreams;
        NewConnection->Registration = QListener->Registration;
        NewConnection->Connection = Event->NEW_CONNECTION.Connection;

        QUIC_STATUS Status = QListener->Registration->QuicApi->ConnectionSetConfiguration(NewConnection->Connection, QListener->Configuration);
        if(QUIC_FAILED(Status)) {
            free(NewConnection);
            return Status;
        }

        QuicConnBoolean KeepConnection = QListener->Callbacks.NewConnectionCallback(NewConnection, QListener->Callbacks.Context);
        if (!KeepConnection) {
            free(NewConnection);
            return QUIC_STATUS_INVALID_STATE;
        }
    } else if (Event->Type == QUIC_LISTENER_EVENT_STOP_COMPLETE) {
        QListener->Callbacks.StoppedCallback(QListener->Callbacks.Context);
    }

    return QUIC_STATUS_SUCCESS;
}

QuicConnStatus QC_CreateListener(const QuicRegistration* Registration, uint16_t Port, uint8_t* Alpn, uint16_t AlpnLength, uint8_t* PfxBuffer, uint32_t PfxSize, const char* PfxPassword, uint32_t NumStreams, QuicListenerCallbacks* Callbacks, QuicListener** Listener) {
    QUIC_STATUS Status;
    QuicListener* NewListener = malloc(sizeof(QuicListener) + AlpnLength);
    if (!NewListener) {
        return QUIC_STATUS_OUT_OF_MEMORY;
    }
    memset(NewListener, 0, sizeof(*NewListener));
    NewListener->Registration = Registration;
    NewListener->NumStreams = NumStreams;
    NewListener->Port = Port;
    NewListener->AlpnLength = AlpnLength;
    memcpy(NewListener->Alpn, Alpn, AlpnLength);

    QUIC_CREDENTIAL_CONFIG CredConfig;
    memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_PKCS12;
    QUIC_CERTIFICATE_PKCS12 Pkcs12;
    Pkcs12.Asn1Blob = PfxBuffer;
    Pkcs12.Asn1BlobLength = PfxSize;
    Pkcs12.PrivateKeyPassword = PfxPassword;

    CredConfig.CertificatePkcs12 = &Pkcs12;

    QUIC_BUFFER AlpnBuffer;
    AlpnBuffer.Buffer = NewListener->Alpn;
    AlpnBuffer.Length = NewListener->AlpnLength;

    QUIC_SETTINGS Settings;
    Settings.IsSetFlags = 0;
    Settings.IsSet.PeerBidiStreamCount = 1;
    Settings.PeerBidiStreamCount = (uint16_t)NumStreams;
    Settings.IsSet.DatagramReceiveEnabled = 1;
    Settings.DatagramReceiveEnabled = 1;
    Settings.IsSet.IdleTimeoutMs = 1;
    Settings.IdleTimeoutMs = 2000; // TODO make this configurable

    Status = Registration->QuicApi->ConfigurationOpen(Registration->Registration, &AlpnBuffer, 1, &Settings, sizeof(Settings), NewListener, &NewListener->Configuration);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = Registration->QuicApi->ConfigurationLoadCredential(NewListener->Configuration, &CredConfig);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = Registration->QuicApi->ListenerOpen(Registration->Registration, ListenerCallback, NewListener, &NewListener->Listener);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    NewListener->Callbacks = *Callbacks;

    Status = QUIC_STATUS_SUCCESS;
    *Listener = NewListener;
    NewListener = NULL;

Exit:
    if (NewListener) {
        if (NewListener->Listener) {
            NewListener->Registration->QuicApi->ListenerClose(NewListener->Listener);
        }
        if (NewListener->Configuration) {
            NewListener->Registration->QuicApi->ConfigurationClose(NewListener->Configuration);
        }
    }

    return Status;
}

QuicConnStatus QC_StartListener(QuicListener* Listener) {
    QUIC_BUFFER AlpnBuffer;
    AlpnBuffer.Buffer = Listener->Alpn;
    AlpnBuffer.Length = Listener->AlpnLength;
    QUIC_ADDR QuicAddr;
    memset(&QuicAddr, 0, sizeof(QuicAddr));
    QuicAddr.Ipv4.sin_port = BYTESWAPSHORT(Listener->Port);
    return Listener->Registration->QuicApi->ListenerStart(Listener->Listener, &AlpnBuffer, 1, &QuicAddr);
}

void QC_StopListener(QuicListener* Listener) {
    Listener->Registration->QuicApi->ListenerStop(Listener->Listener);
}

void QC_FreeListener(QuicListener* Listener) {
    if (Listener) {
        Listener->Registration->QuicApi->ListenerClose(Listener->Listener);
        Listener->Registration->QuicApi->ConfigurationClose(Listener->Configuration);
        free(Listener);
    }
}

static QUIC_STATUS QUIC_API ConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event) {
    (void)Connection;
    (void)Context;
    (void)Event;

    return QUIC_STATUS_SUCCESS;
}

void QC_SetConnectionContext(QuicConnection* Connection, QuicConnectionCallbacks* Callbacks) {
    Connection->Callbacks = *Callbacks;
    Connection->Registration->QuicApi->SetCallbackHandler(Connection->Connection, (void*)ConnectionCallback, Connection);
}

void QC_ShutdownConnection(QuicConnection* Connection) {
    Connection->Registration->QuicApi->ConnectionShutdown(Connection->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
}

void QC_FreeConnection(QuicConnection* Connection) {
    if (Connection) {
        for (uint32_t i = 0; i < Connection->NumStreams; i++) {
            Connection->Registration->QuicApi->StreamClose(Connection->Streams[i]);
        }
        Connection->Registration->QuicApi->ConnectionClose(Connection->Connection);
        free(Connection);
    }
}
