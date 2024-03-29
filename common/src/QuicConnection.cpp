#include "QuicConnection.h"
#include "QuicApi.h"
#include "QuicApiInternal.h"
#include <wpi/mutex.h>
#include <wpi/timestamp.h>
#include <wpi/Synchronization.h>
#include <cstring>

#ifndef QuicNetByteSwapShort
#define QuicNetByteSwapShort htons
#endif

using namespace qapi;

struct QuicConnection::Impl
{
    QuicConnection *Owner = nullptr;
    HQUIC Connection = nullptr;
    HQUIC Stream = nullptr;
    HQUIC ControlStream = nullptr;
    HQUIC ConsoleStream = nullptr;
    HQUIC Listener = nullptr;
    HQUIC Configuration = nullptr;
    wpi::Event ConnShutdown;
    Callbacks Callbacks;

    Impl(QuicConnection *Ownr)
        : Owner{Ownr}
    {
    }

    QUIC_STATUS ConnCallback(QUIC_CONNECTION_EVENT *Event);
    QUIC_STATUS StreamCallback(QUIC_STREAM_EVENT *Event);
    QUIC_STATUS ControlStreamCallback(QUIC_STREAM_EVENT *Event);
    QUIC_STATUS ListenerCallback(QUIC_LISTENER_EVENT *Event);
    QUIC_STATUS ConsoleStreamCallback(QUIC_STREAM_EVENT *Event);

    ~Impl() noexcept
    {
        if (Listener) {
            MsQuic->ListenerStop(Listener);
            MsQuic->ListenerClose(Listener);

            if (!Connection) {
                // If we don't have a connection after stopping and closing the listener
                // everything is free'd already
                return;
            }

            // Shut down the connection
            MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
            // Wait for connection shutdown event
            wpi::WaitForObject(ConnShutdown.GetHandle());
        }

        if (ConsoleStream) {
            MsQuic->StreamShutdown(ConsoleStream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
            MsQuic->StreamClose(ConsoleStream);
        }
        if (ControlStream)
        {
            MsQuic->StreamShutdown(ControlStream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
            MsQuic->StreamClose(ControlStream);
        }
        if (Stream)
        {
            MsQuic->StreamShutdown(Stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
            MsQuic->StreamClose(Stream);
        }
        if (Connection)
        {
            MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
            MsQuic->ConnectionClose(Connection);
        }
        if (Configuration)
        {
            MsQuic->ConfigurationClose(Configuration);
        }
    }
};

QuicConnection::~QuicConnection() noexcept
{
}

void *QuicConnection::GetStreamHandle() noexcept
{
    return pImpl->Stream;
}

void *QuicConnection::GetControlStreamHandle() noexcept
{
    return pImpl->ControlStream;
}

void *QuicConnection::GetConsoleStreamHandle() noexcept
{
    return pImpl->ConsoleStream;
}

void *QuicConnection::GetConnectionHandle() noexcept
{
    return pImpl->Connection;
}

void QuicConnection::WriteDatagram(std::span<uint8_t> datagram)
{
    QUIC_BUFFER *Buffer = (QUIC_BUFFER *)malloc(sizeof(QUIC_BUFFER) + datagram.size());
    Buffer->Buffer = (uint8_t *)(Buffer + 1);
    Buffer->Length = (uint32_t)datagram.size();
    std::memcpy(Buffer->Buffer, datagram.data(), datagram.size());
    QUIC_STATUS Status = MsQuic->DatagramSend(pImpl->Connection, Buffer, 1, QUIC_SEND_FLAG_DGRAM_PRIORITY, Buffer);
    if (QUIC_FAILED(Status))
    {
        printf("Datagram failed to send\n");
        free(Buffer);
    }
}

void QuicConnection::WriteStream(std::span<uint8_t> data)
{
    QUIC_BUFFER *Buffer = (QUIC_BUFFER *)malloc(sizeof(QUIC_BUFFER) + data.size());
    Buffer->Buffer = (uint8_t *)(Buffer + 1);
    Buffer->Length = (uint32_t)data.size();
    std::memcpy(Buffer->Buffer, data.data(), data.size());
    QUIC_STATUS Status = MsQuic->StreamSend(pImpl->Stream, Buffer, 1, QUIC_SEND_FLAG_NONE, Buffer);
    if (QUIC_FAILED(Status))
    {
        printf("Stream failed to send\n");
        free(Buffer);
    }
}

void QuicConnection::WriteControlStream(std::span<uint8_t> data)
{
    QUIC_BUFFER *Buffer = (QUIC_BUFFER *)malloc(sizeof(QUIC_BUFFER) + data.size());
    Buffer->Buffer = (uint8_t *)(Buffer + 1);
    Buffer->Length = (uint32_t)data.size();
    std::memcpy(Buffer->Buffer, data.data(), data.size());
    QUIC_STATUS Status = MsQuic->StreamSend(pImpl->ControlStream, Buffer, 1, QUIC_SEND_FLAG_NONE, Buffer);
    if (QUIC_FAILED(Status))
    {
        printf("Control stream failed to send\n");
        free(Buffer);
    }
}

void QuicConnection::WriteConsoleStream(std::string_view data)
{
    QUIC_BUFFER *Buffer = (QUIC_BUFFER *)malloc(sizeof(QUIC_BUFFER) + data.size());
    Buffer->Buffer = (uint8_t *)(Buffer + 1);
    Buffer->Length = (uint32_t)data.size();
    std::memcpy(Buffer->Buffer, data.data(), data.size());
    QUIC_STATUS Status = MsQuic->StreamSend(pImpl->ControlStream, Buffer, 1, QUIC_SEND_FLAG_NONE, Buffer);
    if (QUIC_FAILED(Status))
    {
        printf("Control stream failed to send\n");
        free(Buffer);
    }
}

_IRQL_requires_max_(PASSIVE_LEVEL)
    _Function_class_(QUIC_STREAM_CALLBACK) static QUIC_STATUS
    QUIC_API
    ConsoleStreamCallback(
        _In_ HQUIC,
        _In_opt_ void *Context,
        _Inout_ QUIC_STREAM_EVENT *Event)
{
    //printf("Stream Callback %d\n", Event->Type);
    return reinterpret_cast<QuicConnection::Impl *>(Context)->ConsoleStreamCallback(Event);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
    _Function_class_(QUIC_STREAM_CALLBACK) static QUIC_STATUS
    QUIC_API
    ControlStreamCallback(
        _In_ HQUIC,
        _In_opt_ void *Context,
        _Inout_ QUIC_STREAM_EVENT *Event)
{
    //printf("Stream Callback %d\n", Event->Type);
    return reinterpret_cast<QuicConnection::Impl *>(Context)->ControlStreamCallback(Event);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
    _Function_class_(QUIC_STREAM_CALLBACK) static QUIC_STATUS
    QUIC_API
    StreamCallback(
        _In_ HQUIC,
        _In_opt_ void *Context,
        _Inout_ QUIC_STREAM_EVENT *Event)
{
    //printf("Stream Callback %d\n", Event->Type);
    return reinterpret_cast<QuicConnection::Impl *>(Context)->StreamCallback(Event);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
    _Function_class_(QUIC_CONNECTION_CALLBACK) static QUIC_STATUS
    QUIC_API
    ConnCallback(
        _In_ HQUIC,
        _In_opt_ void *Context,
        _Inout_ QUIC_CONNECTION_EVENT *Event)
{
    //printf("Connection Callback %d\n", Event->Type);
    return reinterpret_cast<QuicConnection::Impl *>(Context)->ConnCallback(Event);
}

_IRQL_requires_max_(PASSIVE_LEVEL)
    _Function_class_(QUIC_LISTENER_CALLBACK) static QUIC_STATUS
    QUIC_API
    ListenerCallback(
        _In_ HQUIC,
        _In_opt_ void *Context,
        _Inout_ QUIC_LISTENER_EVENT *Event)
{
    //printf("Listener Callback %d\n", Event->Type);
    return reinterpret_cast<QuicConnection::Impl *>(Context)->ListenerCallback(Event);
}

const QUIC_BUFFER Alpn = {sizeof("frc") - 1, (uint8_t *)"frc"};

QuicConnection::QuicConnection(const char* Host, uint16_t Port, Callbacks Cbs, int32_t* Status) noexcept
{
    pImpl.reset(new (std::nothrow)QuicConnection::Impl{this});
    if (!pImpl) {
        *Status = QUIC_STATUS_OUT_OF_MEMORY;
        return;
    }

    pImpl->Callbacks = std::move(Cbs);

    QUIC_STATUS QStatus = MsQuic->ConnectionOpen(GetRegistration(), ConnCallback, pImpl.get(), &pImpl->Connection);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QUIC_SETTINGS Settings;
    Settings.IsSetFlags = 0;
    // Settings.IsSet.PeerBidiStreamCount = 1;
    // Settings.PeerBidiStreamCount = 1;
    Settings.IsSet.KeepAliveIntervalMs = 1;
    Settings.KeepAliveIntervalMs = 1000;
    Settings.IsSet.DatagramReceiveEnabled = 1;
    Settings.DatagramReceiveEnabled = 1;
    Settings.IsSet.IdleTimeoutMs = 1;
    Settings.IdleTimeoutMs = 2000;

    QStatus = MsQuic->ConfigurationOpen(
        GetRegistration(),
        &Alpn,
        1,
        &Settings,
        sizeof(Settings),
        nullptr,
        &pImpl->Configuration);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QUIC_CREDENTIAL_CONFIG CredConfig;
    std::memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;

    QStatus = MsQuic->ConfigurationLoadCredential(pImpl->Configuration, &CredConfig);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    // BOOLEAN value = TRUE;
    // Status =
    //     MsQuic->SetParam(
    //         pImpl->Connection,
    //         QUIC_PARAM_CONN_DISABLE_1RTT_ENCRYPTION,
    //         sizeof(value),
    //         &value);

    // if (QUIC_FAILED(Status))
    // {
    //     throw std::runtime_error("Failed to disable encryption");
    // }

    QStatus = MsQuic->StreamOpen(pImpl->Connection, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, pImpl.get(), &pImpl->Stream);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->StreamStart(pImpl->Stream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->StreamOpen(pImpl->Connection, QUIC_STREAM_OPEN_FLAG_NONE, ControlStreamCallback, pImpl.get(), &pImpl->ControlStream);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->StreamStart(pImpl->ControlStream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->StreamOpen(pImpl->Connection, QUIC_STREAM_OPEN_FLAG_NONE, ConsoleStreamCallback, pImpl.get(), &pImpl->ConsoleStream);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->StreamStart(pImpl->ConsoleStream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->ConnectionStart(pImpl->Connection,
                                     pImpl->Configuration, QUIC_ADDRESS_FAMILY_UNSPEC,
                                     Host, Port);

    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    *Status = 0;
}

//
// Helper function to convert a hex character to its decimal value.
//
uint8_t
DecodeHexChar(
    _In_ char c
    )
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return 0;
}

//
// Helper function to convert a string of hex characters to a byte buffer.
//
uint32_t
DecodeHexBuffer(
    _In_z_ const char* HexBuffer,
    _In_ uint32_t OutBufferLen,
    _Out_writes_to_(OutBufferLen, return)
        uint8_t* OutBuffer
    )
{
    uint32_t HexBufferLen = (uint32_t)strlen(HexBuffer) / 2;
    if (HexBufferLen > OutBufferLen) {
        return 0;
    }

    for (uint32_t i = 0; i < HexBufferLen; i++) {
        OutBuffer[i] =
            (DecodeHexChar(HexBuffer[i * 2]) << 4) |
            DecodeHexChar(HexBuffer[i * 2 + 1]);
    }

    return HexBufferLen;
}

QuicConnection::QuicConnection(uint16_t Port, Callbacks Cbs, int32_t* Status) noexcept
{
    pImpl.reset(new (std::nothrow)QuicConnection::Impl{this});
    if (!pImpl) {
        *Status = QUIC_STATUS_OUT_OF_MEMORY;
        return;
    }
    pImpl->Callbacks = std::move(Cbs);

    QUIC_SETTINGS Settings;
    Settings.IsSetFlags = 0;
    Settings.IsSet.PeerBidiStreamCount = 1;
    Settings.PeerBidiStreamCount = 2;
    // Settings.IsSet.KeepAliveIntervalMs = 1;
    // Settings.KeepAliveIntervalMs = 1000;
    Settings.IsSet.DatagramReceiveEnabled = 1;
    Settings.DatagramReceiveEnabled = 1;
    Settings.IsSet.IdleTimeoutMs = 1;
    Settings.IdleTimeoutMs = 2000;

    QUIC_STATUS QStatus = MsQuic->ConfigurationOpen(
        GetRegistration(),
        &Alpn,
        1,
        &Settings,
        sizeof(Settings),
        nullptr,
        &pImpl->Configuration);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QUIC_CREDENTIAL_CONFIG CredConfig;
    std::memset(&CredConfig, 0, sizeof(CredConfig));
#ifdef _WIN32
    // 601BD997ECE8DFF54EB1B3428A9DF0F630A0DEA1
    CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH;
    QUIC_CERTIFICATE_HASH CertHash;
    DecodeHexBuffer(
                "601BD997ECE8DFF54EB1B3428A9DF0F630A0DEA1",
                sizeof(CertHash.ShaHash),
                CertHash.ShaHash);
    CredConfig.CertificateHash = &CertHash;
#else
    CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    QUIC_CERTIFICATE_FILE CertFile;
    CertFile.CertificateFile = "/Users/thad/GitHub/quiccomm/cert/server.cert";
    CertFile.PrivateKeyFile = "/Users/thad/GitHub/quiccomm/cert/server.key";
    CredConfig.CertificateFile = &CertFile;
#endif

    QStatus = MsQuic->ConfigurationLoadCredential(pImpl->Configuration, &CredConfig);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QStatus = MsQuic->ListenerOpen(GetRegistration(), ListenerCallback, pImpl.get(), &pImpl->Listener);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    QUIC_ADDR LocalAddr;
    std::memset(&LocalAddr, 0, sizeof(LocalAddr));
    LocalAddr.Ipv4.sin_port = QuicNetByteSwapShort(Port);

    QStatus = MsQuic->ListenerStart(pImpl->Listener, &Alpn, 1, &LocalAddr);
    if (QUIC_FAILED(QStatus))
    {
        *Status = (int32_t)QStatus;
        return;
    }

    *Status = 0;
}

QuicConnection::QuicConnection() noexcept = default;

void QuicConnection::Disconnect()
{
    if (pImpl->Stream)
    {
        MsQuic->StreamShutdown(pImpl->Stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
    }

    if (pImpl->ControlStream)
    {
        MsQuic->StreamShutdown(pImpl->ControlStream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
    }

    if (pImpl->Connection) {
        MsQuic->ConnectionShutdown(pImpl->Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
}

QUIC_STATUS QuicConnection::Impl::ConnCallback(QUIC_CONNECTION_EVENT *Event)
{
    if (Event->Type == QUIC_CONNECTION_EVENT_CONNECTED)
    {
        if (ControlStream != nullptr)
        {
            Callbacks.Ready();
        }
    }
    else if (Event->Type == QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED)
    {
        if (ConsoleStream)
        {
            return QUIC_STATUS_NOT_SUPPORTED;
        }
        if (ControlStream)
        {
            MsQuic->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream, (void *)::ConsoleStreamCallback, this);
            ConsoleStream = Event->PEER_STREAM_STARTED.Stream;
            Callbacks.Ready();
        }
        if (Stream)
        {
            MsQuic->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream, (void *)::ControlStreamCallback, this);
            ControlStream = Event->PEER_STREAM_STARTED.Stream;
        }
        else
        {
            MsQuic->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream, (void *)::StreamCallback, this);
            Stream = Event->PEER_STREAM_STARTED.Stream;
        }
    }
    else if (Event->Type == QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE)
    {
        Callbacks.Disconnected();
        ConnShutdown.Set();
    }
    else if (Event->Type == QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED)
    {
        DataBuffer Buffer;
        Buffer.Length = Event->DATAGRAM_RECEIVED.Buffer->Length;
        Buffer.Buffer = Event->DATAGRAM_RECEIVED.Buffer->Buffer;
        Callbacks.DatagramReceived(Buffer);
    }
    else if (Event->Type == QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED)
    {
        if (QUIC_DATAGRAM_SEND_STATE_IS_FINAL(Event->DATAGRAM_SEND_STATE_CHANGED.State))
        {
            free(Event->DATAGRAM_SEND_STATE_CHANGED.ClientContext);
        }
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::Impl::ConsoleStreamCallback(QUIC_STREAM_EVENT *Event)
{
    if (Event->Type == QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE)
    {
        if (Stream)
        {
            MsQuic->StreamShutdown(Stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        }
        MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
    else if (Event->Type == QUIC_STREAM_EVENT_RECEIVE)
    {
        const DataBuffer* Buffers = reinterpret_cast<const DataBuffer*>(Event->RECEIVE.Buffers);
        Callbacks.ConsoleStreamReceived(std::span{Buffers, Event->RECEIVE.BufferCount});
    }
    else if (Event->Type == QUIC_STREAM_EVENT_SEND_COMPLETE)
    {
        free(Event->SEND_COMPLETE.ClientContext);
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::Impl::ControlStreamCallback(QUIC_STREAM_EVENT *Event)
{
    if (Event->Type == QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE)
    {
        if (Stream)
        {
            MsQuic->StreamShutdown(Stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        }
        MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
    else if (Event->Type == QUIC_STREAM_EVENT_RECEIVE)
    {
        const DataBuffer* Buffers = reinterpret_cast<const DataBuffer*>(Event->RECEIVE.Buffers);
        Callbacks.ControlStreamReceived(std::span{Buffers, Event->RECEIVE.BufferCount});
    }
    else if (Event->Type == QUIC_STREAM_EVENT_SEND_COMPLETE)
    {
        free(Event->SEND_COMPLETE.ClientContext);
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::Impl::StreamCallback(QUIC_STREAM_EVENT *Event)
{
    if (Event->Type == QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE)
    {
        if (ControlStream)
        {
            MsQuic->StreamShutdown(ControlStream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
        }
        MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
    else if (Event->Type == QUIC_STREAM_EVENT_RECEIVE)
    {
        const DataBuffer* Buffers = reinterpret_cast<const DataBuffer*>(Event->RECEIVE.Buffers);
        Callbacks.StreamReceived(std::span{Buffers, Event->RECEIVE.BufferCount});
    }
    else if (Event->Type == QUIC_STREAM_EVENT_SEND_COMPLETE)
    {
        free(Event->SEND_COMPLETE.ClientContext);
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::Impl::ListenerCallback(QUIC_LISTENER_EVENT *Event)
{
    if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION)
    {
        QUIC_STATUS Status = MsQuic->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection, Configuration);
        if (QUIC_FAILED(Status)) {
           return Status;
        }
        MsQuic->ListenerStop(Listener);
        MsQuic->SetCallbackHandler(Event->NEW_CONNECTION.Connection, (void *)::ConnCallback, this);
        Connection = Event->NEW_CONNECTION.Connection;
        return Status;
    }
    return QUIC_STATUS_SUCCESS;
}
