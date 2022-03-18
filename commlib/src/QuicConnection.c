#include "QuicConnection.h"
#ifdef _WIN32
//
// The conformant preprocessor along with the newest SDK throws this warning for
// a macro in C mode. As users might run into this exact bug, exclude this
// warning here. This is not an MsQuic bug but a Windows SDK bug.
//
#pragma warning(disable : 5105)
#pragma warning(disable : 4200)  // nonstandard extension used: zero-sized array
                                 // in struct/union
#endif
#include "msquic.h"
#include <stdlib.h>

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

#define BYTESWAPSHORT(x) ((uint16_t)((((x)&0x00ff) << 8) | (((x)&0xff00) >> 8)))

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

typedef struct QuicStream {
    HQUIC Stream;
    QuicConnection* Parent;
    uint32_t Index;
} QuicStream;

struct QuicConnection {
    const QuicRegistration* Registration;
    HQUIC Connection;
    HQUIC Configuration;
    QuicConnectionCallbacks Callbacks;
    uint32_t NumStreams;
    uint32_t NumCurrentStreams;
    QuicStream Streams[0];
};

CommLibStatus COMMLIB_API QC_GetRegistration(const char* Name,
                                             CommLibBoolean UseSingleThread,
                                             QuicRegistration** Registration) {
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
        .ExecutionProfile = QUIC_EXECUTION_PROFILE_LOW_LATENCY};
    if (UseSingleThread) {
        RegConfig.ExecutionProfile = QUIC_EXECUTION_PROFILE_TYPE_SCAVENGER;
    }

    Status = NewRegistration->QuicApi->RegistrationOpen(
        &RegConfig, &NewRegistration->Registration);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = QUIC_STATUS_SUCCESS;
    *Registration = NewRegistration;
    NewRegistration = NULL;

Exit:
    if (NewRegistration) {
        if (NewRegistration->Registration) {
            NewRegistration->QuicApi->RegistrationClose(
                NewRegistration->Registration);
        }
        if (NewRegistration->QuicApi) {
            MsQuicClose(NewRegistration->QuicApi);
        }
    }
    return Status;
}

void COMMLIB_API QC_FreeRegistration(QuicRegistration* Registration) {
    if (Registration) {
        Registration->QuicApi->RegistrationClose(Registration->Registration);
        MsQuicClose(Registration->QuicApi);
        free(Registration);
    }
}

static QUIC_STATUS QUIC_API ListenerCallback(HQUIC Listener, void* Context,
                                             QUIC_LISTENER_EVENT* Event) {
    (void)Listener;
    QuicListener* QListener = (QuicListener*)Context;

    if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION) {
        QuicConnection* NewConnection =
            malloc(sizeof(QuicConnection) +
                   (QListener->NumStreams * sizeof(QuicStream)));
        if (!NewConnection) {
            return QUIC_STATUS_OUT_OF_MEMORY;
        }
        memset(NewConnection, 0,
               sizeof(*NewConnection) + QListener->NumStreams);
        NewConnection->NumStreams = QListener->NumStreams;
        NewConnection->Registration = QListener->Registration;
        NewConnection->Connection = Event->NEW_CONNECTION.Connection;
        for (uint32_t i = 0; i < NewConnection->NumStreams; i++) {
            NewConnection->Streams[i].Parent = NewConnection;
            NewConnection->Streams[i].Index = i;
        }

        QUIC_STATUS Status =
            QListener->Registration->QuicApi->ConnectionSetConfiguration(
                NewConnection->Connection, QListener->Configuration);
        if (QUIC_FAILED(Status)) {
            free(NewConnection);
            return Status;
        }

        CommLibBoolean KeepConnection =
            QListener->Callbacks.NewConnectionCallback(
                NewConnection, QListener->Callbacks.Context);
        if (!KeepConnection) {
            free(NewConnection);
            return QUIC_STATUS_INVALID_STATE;
        }
    } else if (Event->Type == QUIC_LISTENER_EVENT_STOP_COMPLETE) {
        QListener->Callbacks.StoppedCallback(QListener->Callbacks.Context);
    }

    return QUIC_STATUS_SUCCESS;
}

CommLibStatus COMMLIB_API QC_CreateListener(
    const QuicRegistration* Registration, uint16_t Port, const uint8_t* Alpn,
    uint16_t AlpnLength, const uint8_t* PfxBuffer, uint32_t PfxSize,
    const char* PfxPassword, uint32_t NumStreams,
    QuicListenerCallbacks* Callbacks, QuicListener** Listener) {
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
    Settings.IdleTimeoutMs = 2000;  // TODO make this configurable

    Status = Registration->QuicApi->ConfigurationOpen(
        Registration->Registration, &AlpnBuffer, 1, &Settings, sizeof(Settings),
        NewListener, &NewListener->Configuration);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = Registration->QuicApi->ConfigurationLoadCredential(
        NewListener->Configuration, &CredConfig);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = Registration->QuicApi->ListenerOpen(Registration->Registration,
                                                 ListenerCallback, NewListener,
                                                 &NewListener->Listener);
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
            NewListener->Registration->QuicApi->ListenerClose(
                NewListener->Listener);
        }
        if (NewListener->Configuration) {
            NewListener->Registration->QuicApi->ConfigurationClose(
                NewListener->Configuration);
        }
    }

    return Status;
}

CommLibStatus COMMLIB_API QC_StartListener(QuicListener* Listener) {
    QUIC_BUFFER AlpnBuffer;
    AlpnBuffer.Buffer = Listener->Alpn;
    AlpnBuffer.Length = Listener->AlpnLength;
    QUIC_ADDR QuicAddr;
    memset(&QuicAddr, 0, sizeof(QuicAddr));
    QuicAddr.Ipv4.sin_port = BYTESWAPSHORT(Listener->Port);
    return Listener->Registration->QuicApi->ListenerStart(
        Listener->Listener, &AlpnBuffer, 1, &QuicAddr);
}

void COMMLIB_API QC_StopListener(QuicListener* Listener) {
    Listener->Registration->QuicApi->ListenerStop(Listener->Listener);
}

void COMMLIB_API QC_FreeListener(QuicListener* Listener) {
    if (Listener) {
        Listener->Registration->QuicApi->ListenerClose(Listener->Listener);
        Listener->Registration->QuicApi->ConfigurationClose(
            Listener->Configuration);
        free(Listener);
    }
}

static void OnConnectionReady(QuicConnection* Connection) {
    QuicStream* Stream;
    Connection->Callbacks.ReadyCallback(Connection->Callbacks.Context);
    for (uint32_t i = 0; i < Connection->NumStreams; i++) {
        Stream = &Connection->Streams[i];
        Connection->Registration->QuicApi->StreamReceiveSetEnabled(
            Stream->Stream, TRUE);
    }
}

static QUIC_STATUS QUIC_API StreamCallback(HQUIC Stream, void* Context,
                                           QUIC_STREAM_EVENT* Event) {
    UNREFERENCED_PARAMETER(Stream);
    QuicStream* QStream = (QuicStream*)Context;
    QuicConnection* QConnection = QStream->Parent;

    switch (Event->Type) {
        case QUIC_STREAM_EVENT_START_COMPLETE:
            if (QUIC_FAILED(Event->START_COMPLETE.Status)) {
                return QUIC_STATUS_INVALID_STATE;
            }

            QConnection->NumCurrentStreams++;
            if (QConnection->NumCurrentStreams == QConnection->NumStreams) {
                OnConnectionReady(QConnection);
            }
            break;
        case QUIC_STREAM_EVENT_RECEIVE:
            if (QConnection->NumStreams != QConnection->NumCurrentStreams) {
                Event->RECEIVE.TotalBufferLength = 0;
                return QUIC_STATUS_SUCCESS;
            }
            QConnection->Callbacks.StreamDataCallback(
                QConnection->Callbacks.Context, QStream->Index,
                (const QuicDataBuffer*)Event->RECEIVE.Buffers,
                Event->RECEIVE.BufferCount);
            break;
        case QUIC_STREAM_EVENT_SEND_COMPLETE:
            free(Event->SEND_COMPLETE.ClientContext);
            break;
        case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
            // Shutdown connection if a stream ever shuts down.
            QConnection->Registration->QuicApi->ConnectionShutdown(
                QConnection->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
            break;
        default:
            break;
    }
    return QUIC_STATUS_SUCCESS;
}

static QUIC_STATUS QUIC_API ConnectionCallback(HQUIC Connection, void* Context,
                                               QUIC_CONNECTION_EVENT* Event) {
    UNREFERENCED_PARAMETER(Connection);
    QuicConnection* QConnection = (QuicConnection*)Context;
    QuicStream* Stream;
    QUIC_STATUS ParamStatus;
    uint32_t BufferLength;
    QUIC_UINT62 StreamId;

    switch (Event->Type) {
        case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED:
            // Ignore if we haven't sent ready event
            if (QConnection->NumStreams == QConnection->NumCurrentStreams) {
                QConnection->Callbacks.DatagramDataCallback(
                    QConnection->Callbacks.Context,
                    (const QuicDataBuffer*)Event->DATAGRAM_RECEIVED.Buffer);
            }
            break;
        case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
            if (QUIC_DATAGRAM_SEND_STATE_IS_FINAL(
                    Event->DATAGRAM_SEND_STATE_CHANGED.State)) {
                free(Event->DATAGRAM_SEND_STATE_CHANGED.ClientContext);
            }
            break;
        case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
            QConnection->Callbacks.DisconnectedCallback(
                QConnection->Callbacks.Context);
            break;
        case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
            if (QConnection->NumCurrentStreams == QConnection->NumStreams) {
                return QUIC_STATUS_INVALID_STATE;
            }

            BufferLength = sizeof(StreamId);
            ParamStatus = QConnection->Registration->QuicApi->GetParam(
                Event->PEER_STREAM_STARTED.Stream, QUIC_PARAM_STREAM_ID,
                &BufferLength, &StreamId);
            if (QUIC_FAILED(ParamStatus)) {
                return ParamStatus;
            }

            if ((StreamId & 0x3) != 0) {
                return QUIC_STATUS_INVALID_STATE;
            }

            StreamId >>= 2;
            if (StreamId >= QConnection->NumStreams) {
                return QUIC_STATUS_INVALID_STATE;
            }

            Stream = &QConnection->Streams[StreamId];
            Stream->Stream = Event->PEER_STREAM_STARTED.Stream;
            QConnection->Registration->QuicApi->SetCallbackHandler(
                Stream->Stream, (void*)StreamCallback, Stream);
            QConnection->NumCurrentStreams++;

            if (QConnection->NumCurrentStreams == QConnection->NumStreams) {
                OnConnectionReady(QConnection);
            }
            break;
        default:
            break;
    }

    return QUIC_STATUS_SUCCESS;
}

CommLibStatus COMMLIB_API QC_CreateClientConnection(
    const QuicRegistration* Registration, const char* Host, uint16_t Port,
    const uint8_t* Alpn, uint16_t AlpnLength, uint32_t NumStreams,
    CommLibBoolean ValidateCertificate, QuicConnectionCallbacks* Callbacks,
    QuicConnection** Connection) {
    QUIC_STATUS Status;

    const QUIC_BUFFER AlpnBuffer = {.Buffer = (uint8_t*)Alpn,
                                    .Length = AlpnLength};
    QUIC_SETTINGS Settings;
    QUIC_CREDENTIAL_CONFIG CredConfig;
    QuicStream* Stream;
    QuicConnection* NewConnection =
        malloc(sizeof(QuicConnection) + (NumStreams * sizeof(QuicStream)));
    if (!NewConnection) {
        return QUIC_STATUS_OUT_OF_MEMORY;
    }
    memset(NewConnection, 0, sizeof(*NewConnection) + NumStreams);
    NewConnection->NumStreams = NumStreams;
    NewConnection->Registration = Registration;
    NewConnection->Callbacks = *Callbacks;
    for (uint32_t i = 0; i < NewConnection->NumStreams; i++) {
        NewConnection->Streams[i].Parent = NewConnection;
        NewConnection->Streams[i].Index = i;
    }

    Settings.IsSetFlags = 0;
    Settings.IsSet.KeepAliveIntervalMs = 1;
    Settings.KeepAliveIntervalMs = 1000;  // TODO Make this configurable
    Settings.IsSet.DatagramReceiveEnabled = 1;
    Settings.DatagramReceiveEnabled = 1;
    Settings.IsSet.IdleTimeoutMs = 1;
    Settings.IdleTimeoutMs = 2000;  // TODO Make this configurable

    Status = Registration->QuicApi->ConfigurationOpen(
        Registration->Registration, &AlpnBuffer, 1, &Settings, sizeof(Settings),
        NULL, &NewConnection->Configuration);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT |
                       QUIC_CREDENTIAL_FLAG_SET_ALLOWED_CIPHER_SUITES;
    CredConfig.AllowedCipherSuites =
        QUIC_ALLOWED_CIPHER_SUITE_CHACHA20_POLY1305_SHA256;
    if (!ValidateCertificate) {
        CredConfig.Flags |= QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;
    }

    Status = Registration->QuicApi->ConfigurationLoadCredential(
        NewConnection->Configuration, &CredConfig);
    if (QUIC_FAILED(Status)) {
        // Likely schannel, try removing cipher suite
        CredConfig.Flags &= ~QUIC_CREDENTIAL_FLAG_SET_ALLOWED_CIPHER_SUITES;
        CredConfig.AllowedCipherSuites = 0;
        Status = Registration->QuicApi->ConfigurationLoadCredential(
            NewConnection->Configuration, &CredConfig);
        if (QUIC_FAILED(Status)) {
            goto Exit;
        }
    }

    Status = Registration->QuicApi->ConnectionOpen(
        Registration->Registration, ConnectionCallback, NewConnection,
        &NewConnection->Connection);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    for (uint32_t i = 0; i < NumStreams; i++) {
        Stream = &NewConnection->Streams[i];
        Status = Registration->QuicApi->StreamOpen(
            NewConnection->Connection, QUIC_STREAM_OPEN_FLAG_NONE,
            StreamCallback, Stream, &Stream->Stream);
        if (QUIC_FAILED(Status)) {
            goto Exit;
        }

        Status = Registration->QuicApi->StreamStart(
            Stream->Stream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    }

    Status = Registration->QuicApi->ConnectionStart(
        NewConnection->Connection, NewConnection->Configuration,
        QUIC_ADDRESS_FAMILY_UNSPEC, Host, Port);
    if (QUIC_FAILED(Status)) {
        goto Exit;
    }

    Status = QUIC_STATUS_SUCCESS;
    *Connection = NewConnection;
    NewConnection = NULL;

Exit:
    if (NewConnection) {
        for (uint32_t i = 0; i < NewConnection->NumStreams; i++) {
            if (NewConnection->Streams[i].Stream) {
                Registration->QuicApi->StreamClose(
                    NewConnection->Streams[i].Stream);
            }
        }
        if (NewConnection->Connection) {
            Registration->QuicApi->ConnectionClose(NewConnection->Connection);
        }
        if (NewConnection->Configuration) {
            Registration->QuicApi->ConfigurationClose(
                NewConnection->Configuration);
        }
        free(NewConnection);
    }

    return Status;
}

void COMMLIB_API QC_SetConnectionContext(QuicConnection* Connection,
                                         QuicConnectionCallbacks* Callbacks) {
    Connection->Callbacks = *Callbacks;
    Connection->Registration->QuicApi->SetCallbackHandler(
        Connection->Connection, (void*)ConnectionCallback, Connection);
}

void COMMLIB_API QC_ShutdownConnection(QuicConnection* Connection) {
    Connection->Registration->QuicApi->ConnectionShutdown(
        Connection->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
}

void COMMLIB_API QC_FreeConnection(QuicConnection* Connection) {
    if (Connection) {
        for (uint32_t i = 0; i < Connection->NumStreams; i++) {
            if (Connection->Streams[i].Stream) {
                Connection->Registration->QuicApi->StreamClose(
                    Connection->Streams[i].Stream);
            }
        }
        Connection->Registration->QuicApi->ConnectionClose(
            Connection->Connection);
        if (Connection->Configuration) {
            Connection->Registration->QuicApi->ConfigurationClose(
                Connection->Configuration);
        }
        free(Connection);
    }
}
