#include "netcomm.h"

#include "QuicApi.h"
#include <exception>
#include <atomic>
#include <mutex>
#include "remoteconnection.h"

using namespace ncom;
using namespace qapi;

struct Netcomm::Impl {
    Netcomm* Owner;
    HQUIC Registration = nullptr;
    HQUIC Listener = nullptr;
    HQUIC Configuration = nullptr;
    std::promise<std::shared_ptr<RemoteConnection>> ConnPromise;

    std::shared_ptr<RemoteConnection> MakeRemoteConnection(HQUIC ConnHandle) {
        return Owner->MakeRemoteConnection(ConnHandle);
    }

    Impl(Netcomm* owner) : Owner{owner} {

    }

    ~Impl() {
        if (Listener) {
            MsQuic->ListenerStop(Listener);
            MsQuic->ListenerClose(Listener);
        }
        if (Configuration) {
            MsQuic->ConfigurationClose(Configuration);
        }
        if (Registration) {
            MsQuic->RegistrationShutdown(Registration, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
            MsQuic->RegistrationClose(Registration);
        }
    }
};

_IRQL_requires_max_(PASSIVE_LEVEL)
_Function_class_(QUIC_LISTENER_CALLBACK)
static
QUIC_STATUS
QUIC_API
ListenerCallback(
    _In_ HQUIC Listener,
    _In_opt_ void* Context,
    _Inout_ QUIC_LISTENER_EVENT* Event
    )
{
    Netcomm::Impl* impl = reinterpret_cast<Netcomm::Impl*>(Context);
    if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION) {
        try {
            impl->ConnPromise.set_value(impl->MakeRemoteConnection(Event->NEW_CONNECTION.Connection));
        } catch(std::exception&) {
            return QUIC_STATUS_INTERNAL_ERROR;
        }
        MsQuic->ListenerStop(Listener);
    }
    return QUIC_STATUS_SUCCESS;
}

Netcomm::Netcomm() {
    if (!::qapi::InitializeMsQuic()) {
        throw std::runtime_error("Failed to load msquic");
    }

    pImpl = std::make_unique<Impl>(this);

    const QUIC_REGISTRATION_CONFIG RegConfig = {
        "Netcomm",
        QUIC_EXECUTION_PROFILE::QUIC_EXECUTION_PROFILE_LOW_LATENCY
    };

    QUIC_STATUS Status = MsQuic->RegistrationOpen(&RegConfig, &pImpl->Registration);

    if (QUIC_FAILED(Status)) {
        throw std::runtime_error("Failed to open registration");
    }

    Status = MsQuic->ListenerOpen(pImpl->Registration, ListenerCallback, this->pImpl.get(), &pImpl->Listener);

    if (QUIC_FAILED(Status)) {
        throw std::runtime_error("Failed to open listener");
    }
}

Netcomm::~Netcomm() noexcept {
    pImpl = nullptr;
    ::qapi::FreeMsQuic();
}

const QUIC_BUFFER Alpn = { sizeof("frc") - 1, (uint8_t*)"frc" };

std::future<std::shared_ptr<RemoteConnection>> Netcomm::StartListener() {
    QUIC_ADDR Addr = {0};
    Addr.Ipv4.sin_port = htons(1188);
    MsQuic->ListenerStart(pImpl->Listener, &Alpn, 1, &Addr);
    printf("Listener started\n");
    pImpl->ConnPromise = {};
    return pImpl->ConnPromise.get_future();
}

std::shared_ptr<RemoteConnection> Netcomm::MakeRemoteConnection(void* ConnHandle) {
    return std::make_shared<RemoteConnection>(RemoteConnection::private_init{}, ConnHandle);
}
