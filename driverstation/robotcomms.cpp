#include "robotcomms.h"
#include <atomic>
#include <thread>
#include <wpi/Synchronization.h>
#include <wpi/StringExtras.h>
#include <wpi/mutex.h>
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wunused-parameter"
#else
#pragma warning(disable:4458)
#endif
#include <wpi/SmallVector.h>
#include <QuicConnection.h>
#include "controlpacketthread.h"

using namespace ds;

struct RobotComms::Impl
{
    Impl(ds::DsEvents* DsEvents, DS_Status *Status) noexcept;
    ~Impl() noexcept;

    void ThreadMain();

    wpi::Event DisconnectedEvent;
    wpi::Event ReadyEvent;

    char Hostname[257];

    wpi::mutex HostnameMutex;
    wpi::Event HostnameUpdateEvent;

    ds::DsEvents* DsEvents;
    std::atomic_bool ThreadRunning{true};
    std::thread Thread;

    void Ready() {
        ReadyEvent.Set();
    }
    void Disconnected() {
        DisconnectedEvent.Set();
    }
};

RobotComms::Impl::Impl(ds::DsEvents* Events, DS_Status *Status) noexcept
 : DsEvents{Events}, Thread{[&] {ThreadMain();}} {
     (void)Status;
}

RobotComms::Impl::~Impl() noexcept {
    ThreadRunning = false;
    HostnameUpdateEvent.Set();
    if (Thread.joinable()) {
        Thread.join();
    }
}

struct QuicRobotConnection {
    qapi::QuicConnection Connection;
    ds::ControlPacketThread ControlPacketThread;

    QuicRobotConnection(ds::ControlPacketThread::Callback ControlCallback, const char* Hostname, qapi::Callbacks QuicCallbacks, int32_t* Status)
        : Connection{Hostname, 1360, std::move(QuicCallbacks), Status},
            ControlPacketThread{std::move(ControlCallback), Status} {
    }
};

void RobotComms::Impl::ThreadMain() {
    qapi::Callbacks Callbacks = DsEvents->GetQuicCallbacks();
    Callbacks.Ready = [&]{Ready();};
    Callbacks.Disconnected = [&]{Disconnected();};

    wpi::SmallVector<WPI_Handle, 64> Events;
    wpi::SmallVector<WPI_Handle, 64> SignaledEventsStorage;
    std::span<WPI_Handle> SignaledEvents;

    std::unique_ptr<QuicRobotConnection> RobotConnection;

    while (ThreadRunning) {
        Events.clear();
        SignaledEventsStorage.clear();

        Events.emplace_back(HostnameUpdateEvent);
        Events.emplace_back(ReadyEvent);
        Events.emplace_back(DisconnectedEvent);

        SignaledEventsStorage.resize(Events.size());
        SignaledEvents = wpi::WaitForObjects(Events, SignaledEventsStorage);

        if (!ThreadRunning) {
            break;
        }

        for (auto&& Event : SignaledEvents) {
            if (Event == HostnameUpdateEvent.GetHandle()) {
                if (RobotConnection) {
                    RobotConnection->Connection.Disconnect();
                } else {
                    RobotConnection.reset(new (std::nothrow)QuicRobotConnection{})
                }
            } else if (Event == ReadyEvent.GetHandle()) {
                // Ready

                // Configure control thread
            } else {
                // Disconnect

                // Free Control Thread

            }
        }
    }
}

RobotComms::RobotComms(ds::DsEvents* DsEvents, DS_Status *Status) noexcept
{
    *Status = DS_OUT_OF_MEMORY;
    pImpl.reset(new (std::nothrow) Impl{DsEvents, Status});
    if (*Status != DS_STATUS_SUCCESS)
    {
        return;
    }
}

RobotComms::~RobotComms() noexcept
{
    pImpl = nullptr;
}

bool RobotComms::SetTeam(const char *Team) noexcept
{
    int ToCopyLength = 0;
    auto TeamAsInt = wpi::parse_integer<uint32_t>(Team, 10);
    char Buf[257];
    if (TeamAsInt.has_value())
    {
        int Result = snprintf(Buf, 257, "roborio-%u-frc.local", TeamAsInt.value());
        if (Result <= 0 || Result >= 257)
        {
            return false;
        }
        Buf[Result] = '\0';
        ToCopyLength = Result + 1;
    } else {
        size_t TeamLength = strlen(Team);
        if (TeamLength > 256) {
            return false;
        }
        std::copy_n(Team, TeamLength, Buf);
        Buf[TeamLength] = '\0';
        ToCopyLength = (int)TeamLength + 1;
    }

    std::scoped_lock Lock{pImpl->HostnameMutex};
    std::copy_n(Buf, ToCopyLength, pImpl->Hostname);
    pImpl->HostnameUpdateEvent.Set();
    return true;
}
