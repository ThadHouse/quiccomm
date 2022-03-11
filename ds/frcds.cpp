#include "frcds.h"
#include "QuicConnection.h"
#include "QuicApi.h"
#include <thread>
#include <wpi/mutex.h>
#include <fmt/format.h>
#include <atomic>
#include <wpi/timestamp.h>
#include <QuicApiInternal.h>
#include "tags/JoystickData.h"
#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wunused-parameter"
#else
#pragma warning(disable:4458)
#endif
#include <wpi/SmallVector.h>

using namespace ds;
using namespace qapi;

struct DriverStation::Impl
{
    DriverStation* Owner;
    QuicApi Api{"DriverStation"};
    std::array<QuicConnection, 4> ConnectionStore;
    int ConnectionCount = 0;
    std::vector<QuicConnection *> PendingConnections;
    QuicConnection *CurrentConnectionInternal = nullptr;
    QuicConnection* CurrentConnection = nullptr;
    std::thread EventThread;
    std::atomic_bool ThreadRunning{true};
    wpi::Event EndThreadEvent;
    wpi::Event TeamNumberChangeEvent;
    wpi::Event AckDisconnectEvent;

    wpi::mutex AppDataMutex;
    std::string Hostname;
    int TeamNumber = 0;
    uint64_t LastCheckTime = 0;

    void ThreadRun();
    void InitializeConnections();
    void HandleStreamData();
    void HandleControlStreamData();
    void HandleDatagramData();
    void HandleTeamNumberChange();
    void HandleDisconnect();

    Impl(DriverStation* Owner) : Owner{Owner}, EventThread{[&]
                         { ThreadRun(); }}
    {
    }

    ~Impl()
    {
        ThreadRunning = false;
        EndThreadEvent.Set();
        if (EventThread.joinable())
            EventThread.join();
    }
};

void DriverStation::Impl::ThreadRun()
{
    wpi::SmallVector<WPI_Handle, 64> Events;
    wpi::SmallVector<WPI_Handle, 64> SignaledEventsStorage;
    wpi::span<WPI_Handle> SignaledEvents;

    // First, wait for init or exit
    Events.emplace_back(EndThreadEvent);
    Events.emplace_back(TeamNumberChangeEvent);
    SignaledEventsStorage.resize(Events.size());

    wpi::WaitForObjects(Events, SignaledEventsStorage);
    // No need to actually check signaled events, as we will either have received
    // started, or the thread being killed.

    while (ThreadRunning)
    {
        Events.clear();
        SignaledEventsStorage.clear();
        Events.emplace_back(EndThreadEvent);
        Events.emplace_back(TeamNumberChangeEvent);
        Events.emplace_back(AckDisconnectEvent);

        if (CurrentConnectionInternal != nullptr)
        {
            Events.emplace_back(CurrentConnectionInternal->GetDisconnectedEvent());
            Events.emplace_back(CurrentConnectionInternal->GetReadyEvent());
            Events.emplace_back(CurrentConnectionInternal->GetDatagramEvent());
            Events.emplace_back(CurrentConnectionInternal->GetStreamEvent());
            Events.emplace_back(CurrentConnectionInternal->GetControlStreamEvent());
        }
        else
        {
            // If all are null, or we are empty,
            // we need to trigger a new connection
            bool allEmpty = true;
            for (auto &&i : PendingConnections)
            {
                if (i != nullptr)
                {
                    allEmpty = false;
                    break;
                }
            }
            if (allEmpty)
            {
                InitializeConnections();
            }

            // Add each connection event
            for (auto &&i : PendingConnections)
            {
                if (i == nullptr)
                {
                    continue;
                }
                allEmpty = false;
                Events.emplace_back(i->GetDisconnectedEvent());
                Events.emplace_back(i->GetReadyEvent());
                Events.emplace_back(i->GetDatagramEvent());
                Events.emplace_back(i->GetStreamEvent());
                Events.emplace_back(i->GetControlStreamEvent());
            }
        }

        SignaledEventsStorage.resize(Events.size());
        if (CurrentConnectionInternal)
        {
            SignaledEvents = wpi::WaitForObjects(Events, SignaledEventsStorage);
        }
        else
        {
            bool TimedOut = false;
            SignaledEvents = wpi::WaitForObjects(Events, SignaledEventsStorage, 1.2, &TimedOut);
            if (!ThreadRunning)
            {
                break;
            }
            if (TimedOut)
            {
                continue;
            }
        }

        for (auto &&i : SignaledEvents)
        {
            if (i == EndThreadEvent.GetHandle())
            {
                break;
            }
            else if (i == TeamNumberChangeEvent.GetHandle())
            {
                HandleTeamNumberChange();
                break;
            } else if (i == AckDisconnectEvent.GetHandle()) {
                if (CurrentConnectionInternal == nullptr) {
                    CurrentConnection = nullptr;
                }
                break;
            }

            if (CurrentConnectionInternal != nullptr)
            {
                if (CurrentConnectionInternal->GetDisconnectedEvent() == i)
                {
                    HandleDisconnect();
                    break;
                }
                else if (CurrentConnectionInternal->GetStreamEvent() == i)
                {
                    HandleStreamData();
                    break;
                }
                else if (CurrentConnectionInternal->GetControlStreamEvent() == i)
                {
                    HandleControlStreamData();
                    break;
                }
                else if (CurrentConnectionInternal->GetDatagramEvent() == i)
                {
                    HandleDatagramData();
                    break;
                }
            }
            else
            {
                bool HandledEvent = false;
                for (auto &&j : PendingConnections)
                {
                    if (j == nullptr)
                    {
                        continue;
                    }
                    if (j->GetDisconnectedEvent() == i)
                    {
                        j = nullptr;
                        HandledEvent = true;
                        break;
                    }
                    else if (j->GetReadyEvent() == i)
                    {
                        CurrentConnectionInternal = j;
                        CurrentConnection = j;
                        for (auto &&k : PendingConnections)
                        {
                            if (k != CurrentConnectionInternal)
                            {
                                k->Disconnect();
                            }
                        }
                        HandledEvent = true;
                        std::scoped_lock lock{Owner->EventMutex};
                        Owner->Events.emplace_back(DsEvent::ConnectedEvent());
                        Owner->Event.Set();
                        break;
                    }
                }
                if (HandledEvent)
                {
                    break;
                }
            }
        }
    }
}

void DriverStation::Impl::InitializeConnections()
{
    if (CurrentConnection) {
        return;
    }

    auto Now = wpi::Now();
    if (Now - LastCheckTime < 1000000)
    {
        return;
    }
    LastCheckTime = Now;

    std::string ConnectAddr;
    {
        std::scoped_lock lock{AppDataMutex};
        if (Hostname.empty())
        {
            ConnectAddr = fmt::format("roborio-{}-frc.local", TeamNumber);
        }
        else
        {
            ConnectAddr = Hostname;
        }
    }

    PendingConnections.clear();
    ConnectionCount = 0;

    // TODO handle if connection count ever is greater then 4
    ConnectionStore[ConnectionCount].~QuicConnection();// = QuicConnection{ConnectAddr, 1360};
    QuicConnection* qc = new (&ConnectionStore[ConnectionCount])(QuicConnection)(ConnectAddr, 1360);
    PendingConnections.emplace_back(qc);
    ConnectionCount++;
}

void DriverStation::Impl::HandleStreamData()
{
    printf("Received Stream Data\n");
    std::vector<uint8_t> StreamData;
    CurrentConnectionInternal->GetStreamData(StreamData);
}

void DriverStation::Impl::HandleControlStreamData()
{
    printf("Received Control Stream Data\n");
        std::vector<uint8_t> StreamData;
    CurrentConnectionInternal->GetControlStreamData(StreamData);
}

void DriverStation::Impl::HandleDatagramData()
{
    printf("Received Datagram Data\n");
    std::vector<DatagramBuffer> Buffers;
    CurrentConnectionInternal->GetDatagramData(Buffers);
}

void DriverStation::Impl::HandleTeamNumberChange()
{
    for (auto &&i : PendingConnections)
    {
        if (i != nullptr)
        {
            i->Disconnect();
        }
    }
}

void DriverStation::Impl::HandleDisconnect()
{
    CurrentConnectionInternal->Disconnect();
    CurrentConnectionInternal = nullptr;
    for (auto&& i : PendingConnections) {
        if (i == CurrentConnection) {
            i = nullptr;
        }
    }
    std::scoped_lock lock{Owner->EventMutex};
    Owner->Events.emplace_back(DsEvent::DisconnectedEvent());
    Owner->Event.Set();
}

DriverStation::DriverStation()
{
    pImpl = std::make_unique<Impl>(this);
}

DriverStation::~DriverStation() noexcept
{
}

void DriverStation::SetTeamNumber(int teamNumber)
{
    std::scoped_lock lock{pImpl->AppDataMutex};
    pImpl->Hostname = "";
    pImpl->TeamNumber = teamNumber;
    pImpl->TeamNumberChangeEvent.Set();
}
void DriverStation::SetHostname(std::string host)
{
    std::scoped_lock lock{pImpl->AppDataMutex};
    pImpl->Hostname = std::move(host);
    pImpl->TeamNumberChangeEvent.Set();
}

void DriverStation::AckDisconnect() {
    pImpl->AckDisconnectEvent.Set();
}

DsEvent DsEvent::ConnectedEvent() noexcept
{
    return DsEvent{DS_Event_Connected};
}

DsEvent DsEvent::DisconnectedEvent() noexcept
{
    return DsEvent{DS_Event_Disconnected};
}

uint8_t cpCount = 0;
uint8_t gdCount = 0;

void DriverStation::SendControlPacket() {
    if (!pImpl->CurrentConnection) {
        return;
    }
    wpi::SmallVector<char, 512> Vec;
    wpi::raw_svector_ostream svec{Vec};

    COMM_JoystickData cJoyData = {};
    cJoyData.isConnected = true;
    cJoyData.axesCount = 1;
    cJoyData.axes[0] = 42;
    tags::JoystickData jData{cJoyData};
    jData.WriteTag(svec);

    wpi::span<uint8_t> Data{(uint8_t*)Vec.data(), Vec.size()};

    pImpl->CurrentConnection->WriteDatagram(Data);
}

void DriverStation::SendGameData() {
    if (!pImpl->CurrentConnection) {
        return;
    }
    uint8_t Data[43];
    Data[0] = gdCount;
    gdCount++;
    pImpl->CurrentConnection->WriteStream(Data);
}


uint32_t DriverStation::GetRtt() noexcept{
    if (!pImpl->CurrentConnection) {
        return 0;
    }
    QUIC_STATISTICS_V2 Stats;
    Stats.Rtt = 0;
    uint32_t StatsLen = sizeof(Stats);
    qapi::GetApiTable()->GetParam((HQUIC)pImpl->CurrentConnection->GetConnectionHandle(), QUIC_PARAM_CONN_STATISTICS_V2, &StatsLen, &Stats);
    return Stats.Rtt;
}
