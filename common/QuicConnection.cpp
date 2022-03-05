#include "QuicConnection.h"
#include "QuicApi.h"
#include "QuicApiInternal.h"

using namespace qapi;

struct QuicConnection::Impl
{
    QuicConnection *Owner = nullptr;
    HQUIC Connection = nullptr;
    HQUIC Stream = nullptr;
    HQUIC Listener = nullptr;
    HQUIC Configuration = nullptr;

    wpi::Event ReadyEvent;
    wpi::Event DatagramEvent;
    wpi::Event StreamEvent;
    wpi::Event DisconnectedEvent;

    Impl(QuicConnection *Ownr)
        : Owner{Ownr}
    {
    }

    QUIC_STATUS ConnCallback(QUIC_CONNECTION_EVENT *Event);
    QUIC_STATUS StreamCallback(QUIC_STREAM_EVENT *Event);
    QUIC_STATUS ListenerCallback(QUIC_LISTENER_EVENT* Event);

    ~Impl() noexcept
    {
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
        if (Configuration) {
            MsQuic->ConfigurationClose(Configuration);
        }
        if (Listener) {
            MsQuic->ListenerStop(Listener);
            MsQuic->ListenerClose(Listener);
        }
    }
};

QuicConnection::~QuicConnection() noexcept
{
}

WPI_EventHandle QuicConnection::GetReadyEvent() noexcept {
    return pImpl->ReadyEvent.GetHandle();
}

WPI_EventHandle QuicConnection::GetDatagramEvent() noexcept
{
    return pImpl->DatagramEvent.GetHandle();
}

WPI_EventHandle QuicConnection::GetStreamEvent() noexcept
{
    return pImpl->StreamEvent.GetHandle();
}

WPI_EventHandle QuicConnection::GetDisconnectedEvent() noexcept
{
    return pImpl->DisconnectedEvent.GetHandle();
}

void *QuicConnection::GetStreamHandle() noexcept
{
    return pImpl->Stream;
}

void *QuicConnection::GetConnectionHandle() noexcept
{
    return pImpl->Connection;
}

void QuicConnection::WriteDatagram(wpi::span<uint8_t> datagram)
{
    (void)datagram;
}

void QuicConnection::WriteStream(wpi::span<uint8_t> data)
{
    (void)data;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
    _Function_class_(QUIC_STREAM_CALLBACK) static QUIC_STATUS
    QUIC_API
    StreamCallback(
        _In_ HQUIC,
        _In_opt_ void *Context,
        _Inout_ QUIC_STREAM_EVENT *Event)
{
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
    return reinterpret_cast<QuicConnection::Impl *>(Context)->ListenerCallback(Event);
}

const QUIC_BUFFER Alpn = {sizeof("frc") - 1, (uint8_t *)"frc"};

QuicConnection::QuicConnection(std::string Host, uint16_t Port)
{
    pImpl = std::make_unique<QuicConnection::Impl>(this);
    QUIC_STATUS Status = MsQuic->ConnectionOpen(GetRegistration(), ConnCallback, pImpl.get(), &pImpl->Connection);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to open connection");
    }

    QUIC_SETTINGS Settings;
    Settings.IsSetFlags = 0;
    // Settings.IsSet.PeerBidiStreamCount = 1;
    // Settings.PeerBidiStreamCount = 1;
    Settings.IsSet.KeepAliveIntervalMs = 1;
    Settings.KeepAliveIntervalMs = 1000;
    Settings.IsSet.DatagramReceiveEnabled = 1;
    Settings.DatagramReceiveEnabled = 1;

    Status = MsQuic->ConfigurationOpen(
        GetRegistration(),
        &Alpn,
        1,
        &Settings,
        sizeof(Settings),
        nullptr,
        &pImpl->Configuration);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to open configuration");
    }

    QUIC_CREDENTIAL_CONFIG CredConfig;
    std::memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Flags = QUIC_CREDENTIAL_FLAG_CLIENT | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;

    Status = MsQuic->ConfigurationLoadCredential(pImpl->Configuration, &CredConfig);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to load credential");
    }

    Status = MsQuic->StreamOpen(pImpl->Connection, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, pImpl.get(), &pImpl->Stream);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to open stream");
    }

    Status = MsQuic->StreamStart(pImpl->Stream, QUIC_STREAM_START_FLAG_IMMEDIATE);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to start stream");
    }

    Status = MsQuic->ConnectionStart(pImpl->Connection,
                                     pImpl->Configuration, QUIC_ADDRESS_FAMILY_UNSPEC,
                                     Host.c_str(), Port);

    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to start connection");
    }
}

QuicConnection::QuicConnection(uint16_t Port) {
    pImpl = std::make_unique<QuicConnection::Impl>(this);

    QUIC_SETTINGS Settings;
    Settings.IsSetFlags = 0;
    Settings.IsSet.PeerBidiStreamCount = 1;
    Settings.PeerBidiStreamCount = 1;
    // Settings.IsSet.KeepAliveIntervalMs = 1;
    // Settings.KeepAliveIntervalMs = 1000;
    Settings.IsSet.DatagramReceiveEnabled = 1;
    Settings.DatagramReceiveEnabled = 1;

    QUIC_STATUS Status = MsQuic->ConfigurationOpen(
        GetRegistration(),
        &Alpn,
        1,
        &Settings,
        sizeof(Settings),
        nullptr,
        &pImpl->Configuration);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to open configuration");
    }

    QUIC_CREDENTIAL_CONFIG CredConfig;
    std::memset(&CredConfig, 0, sizeof(CredConfig));
    CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    QUIC_CERTIFICATE_FILE CertFile;
    CertFile.CertificateFile = "/Users/thad/GitHub/quiccomm/cert/server.cert";
    CertFile.PrivateKeyFile = "/Users/thad/GitHub/quiccomm/cert/server.key";
    CredConfig.CertificateFile = &CertFile;

    Status = MsQuic->ConfigurationLoadCredential(pImpl->Configuration, &CredConfig);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to load credential");
    }

    Status = MsQuic->ListenerOpen(GetRegistration(), ListenerCallback, pImpl.get(), &pImpl->Listener);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to open listener");
    }

    QUIC_ADDR LocalAddr;
    std::memset(&LocalAddr, 0, sizeof(LocalAddr));
    LocalAddr.Ipv4.sin_port = htons(Port);

    Status = MsQuic->ListenerStart(pImpl->Listener, &Alpn, 1, &LocalAddr);
    if (QUIC_FAILED(Status))
    {
        throw std::runtime_error("Failed to open listener");
    }
}

QUIC_STATUS QuicConnection::Impl::ConnCallback(QUIC_CONNECTION_EVENT *Event) {
    if (Event->Type == QUIC_CONNECTION_EVENT_CONNECTED) {
        ReadyEvent.Set();
    } else if (Event->Type == QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED) {
        MsQuic->SetCallbackHandler(Event->PEER_STREAM_STARTED.Stream, (void*)::StreamCallback, this);
        Stream = Event->PEER_STREAM_STARTED.Stream;
        ReadyEvent.Set();
    } else if (Event->Type == QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE) {
        DisconnectedEvent.Set();
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::Impl::StreamCallback(QUIC_STREAM_EVENT *Event) {
    if (Event->Type == QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE) {
        MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
    return QUIC_STATUS_SUCCESS;
}

QUIC_STATUS QuicConnection::Impl::ListenerCallback(QUIC_LISTENER_EVENT *Event) {
    if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION) {
        MsQuic->ListenerStop(Listener);
        MsQuic->SetCallbackHandler(Event->NEW_CONNECTION.Connection, (void*)::ConnCallback, this);
        Connection = Event->NEW_CONNECTION.Connection;
        QUIC_STATUS Status = MsQuic->ConnectionSetConfiguration(Event->NEW_CONNECTION.Connection, Configuration);
        return Status;
    }
    return QUIC_STATUS_SUCCESS;
}
