#pragma once

#include "stdint.h"

typedef struct QuicRegistration QuicRegistration;
typedef struct QuicConnection QuicConnection;
typedef struct QuicListener QuicListener;

typedef struct QuicDataBuffer {
    uint32_t Length;
    uint8_t *Buffer;
} QuicDataBuffer;

#ifdef _WIN32
#define QUIC_CONN_API __cdecl
#else
#define QUIC_CONN_API
#endif

typedef int32_t QuicConnStatus;
typedef uint32_t QuicConnBoolean;

typedef QuicConnBoolean(QUIC_CONN_API * QuicListenerNewConnectionCallback)(QuicConnection* Connection, void* Context);
typedef void(QUIC_CONN_API * QuicListenerStoppedCallback)(void* Context);
typedef void(QUIC_CONN_API * QuicConnectionReadyCallback)(void* Context);
typedef void(QUIC_CONN_API * QuicConnectionDisconnectedCallback)(void* Context);
typedef void(QUIC_CONN_API * QuicConnectionDatagramDataCallback)(void* Context, const QuicDataBuffer* DataBuffer);
typedef void(QUIC_CONN_API * QuicConnectionStreamDataCallback)(void* Context, uint32_t StreamId, const QuicDataBuffer* DataBuffers, uint32_t DataBuffersSize);

typedef struct QuicListenerCallbacks {
    QuicListenerNewConnectionCallback NewConnectionCallback;
    QuicListenerStoppedCallback StoppedCallback;
    void* Context;
} QuicListenerCallbacks;

typedef struct QuicConnectionCallbacks {
    QuicConnectionReadyCallback ReadyCallback;
    QuicConnectionDisconnectedCallback DisconnectedCallback;
    QuicConnectionDatagramDataCallback DatagramDataCallback;
    QuicConnectionStreamDataCallback StreamDataCallback;

    void* Context;
} QuicConnectionCallbacks;

#ifdef __cplusplus
extern "C" {
#endif

QuicConnStatus QC_GetRegistration(const char* Name, QuicConnBoolean UseSingleThread, QuicRegistration** Registration);
void QC_FreeRegistration(QuicRegistration* Registration);

QuicConnStatus QC_CreateListener(const QuicRegistration* Registration, uint16_t Port, uint8_t* Alpn, uint16_t AlpnLength, uint8_t* PfxBuffer, uint32_t PfxSize, const char* PfxPassword, uint32_t NumStreams, QuicListenerCallbacks* Callbacks, QuicListener** Listener);
QuicConnStatus QC_StartListener(QuicListener* Listener);
void QC_StopListener(QuicListener* Listener);

// Listener must be stopped, and QuicListenerStopped event must have been received in order to
// call this API. Additionally, all connections received from the listener must have been freed.
void QC_FreeListener(QuicListener* Listener);

void QC_SetConnectionContext(QuicConnection* Connection, QuicConnectionCallbacks* Callbacks);

QuicConnStatus QC_CreateClientConnection(const QuicRegistration* Registration, const char* Host, uint16_t Port, uint8_t* Alpn, uint16_t AlpnLength, uint32_t NumStreams, QuicConnBoolean ValidateCertificate, QuicConnectionCallbacks* Callbacks, QuicConnection** Connection);

void QC_ShutdownConnection(QuicConnection* Connection);
void QC_FreeConnection(QuicConnection* Connection);



#ifdef __cplusplus
}
#endif
