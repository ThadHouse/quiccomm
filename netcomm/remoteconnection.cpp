#include "remoteconnection.h"
#include "QuicApi.h"

using namespace ncom;
using namespace qapi;

struct RemoteConnection::Impl {
    HQUIC Connection = nullptr;
    HQUIC Stream = nullptr;

    std::promise<void> StreamReadyPromise;

    ~Impl() noexcept {
        if (Stream) {
            MsQuic->StreamShutdown(Stream, QUIC_STREAM_SHUTDOWN_FLAG_ABORT, 0);
            MsQuic->StreamClose(Stream);
        }

        if (Connection) {
            MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
            MsQuic->ConnectionClose(Connection);
        }
    }
};

_IRQL_requires_max_(PASSIVE_LEVEL)
_Function_class_(QUIC_STREAM_CALLBACK)
static
QUIC_STATUS
QUIC_API
StreamCallback(
    _In_ HQUIC,
    _In_opt_ void*,
    _Inout_ QUIC_STREAM_EVENT*
    )
{
    return QUIC_STATUS_SUCCESS;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
_Function_class_(QUIC_CONNECTION_CALLBACK)
static
QUIC_STATUS
QUIC_API
ConnCallback(
    _In_ HQUIC Connection,
    _In_opt_ void* Context,
    _Inout_ QUIC_CONNECTION_EVENT* Event
    )
{
    RemoteConnection::Impl* impl = reinterpret_cast<RemoteConnection::Impl*>(Context);
    if (Event->Type == QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED) {
        if (impl->Stream != nullptr) {
            MsQuic->ConnectionShutdown(Connection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 42);
            return QUIC_STATUS_INVALID_STATE;
        }
        impl->Stream = Event->PEER_STREAM_STARTED.Stream;
        MsQuic->SetCallbackHandler(impl->Stream, (void*)StreamCallback, impl);
        impl->StreamReadyPromise.set_value();
    }
    return QUIC_STATUS_SUCCESS;
}

RemoteConnection::RemoteConnection(const private_init&, void* ConnHandle) {
    pImpl = std::make_unique<Impl>();
    pImpl->Connection = static_cast<HQUIC>(ConnHandle);
    MsQuic->SetCallbackHandler(pImpl->Connection, reinterpret_cast<void*>(ConnCallback), pImpl.get());

}

RemoteConnection::~RemoteConnection() noexcept {

}

std::future<void> RemoteConnection::GetStreamReadyFuture() {
    return pImpl->StreamReadyPromise.get_future();
}
