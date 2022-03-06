#include "frcds.h"
#include "QuicConnection.h"
#include "QuicApi.h"
#include <thread>
#include <optional>
#include <wpi/mutex.h>
#include <fmt/format.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <wpi/SmallVector.h>

using namespace ds;
using namespace qapi;

struct DriverStation::Impl
{
    QuicApi Api{"DriverStation"};
    std::array<QuicConnection, 4> ConnectionStore;
    int ConnectionCount;
    std::vector<QuicConnection*> PendingConnections;
    QuicConnection *CurrentConnection = nullptr;
    std::thread EventThread;
    std::atomic_bool ThreadRunning{true};
    wpi::Event EndThreadEvent;
    wpi::Event NewDataEvent{true};

    wpi::mutex AppDataMutex;
    std::string Hostname;
    int TeamNumber = 0;

    void ThreadRun();
    void InitializeConnections();
    void HandleStreamData();
    void HandleDatagramData();
    void HandleNewAppData();
    void HandleDisconnect();

    Impl() : EventThread{[&]
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
    while (ThreadRunning)
    {
        Events.clear();
        SignaledEventsStorage.clear();
        Events.emplace_back(EndThreadEvent);
        Events.emplace_back(NewDataEvent);

        if (CurrentConnection != nullptr)
        {
            Events.emplace_back(CurrentConnection->GetDisconnectedEvent());
            Events.emplace_back(CurrentConnection->GetReadyEvent());
            Events.emplace_back(CurrentConnection->GetDatagramEvent());
            Events.emplace_back(CurrentConnection->GetStreamEvent());
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
            }
        }

        SignaledEventsStorage.resize(Events.size());
        const auto SignaledEvents = wpi::WaitForObjects(Events, SignaledEventsStorage);

        for (auto &&i : SignaledEvents)
        {
            if (i == EndThreadEvent.GetHandle())
            {
                break;
            }
            else if (i == NewDataEvent.GetHandle())
            {
                HandleNewAppData();
                break;
            }

            if (CurrentConnection != nullptr)
            {
                if (CurrentConnection->GetDisconnectedEvent() == i)
                {
                    HandleDisconnect();
                    break;
                }
                else if (CurrentConnection->GetStreamEvent() == i)
                {
                    HandleStreamData();
                    break;
                }
                else if (CurrentConnection->GetDatagramEvent() == i)
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
                        CurrentConnection = j;
                        for (auto&& k : PendingConnections) {
                            if (k != CurrentConnection) {
                                k->Disconnect();
                            }
                        }
                        HandledEvent = true;
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
    std::string ConnectAddr;
    {
        std::scoped_lock lock{AppDataMutex};
        if (Hostname.empty()) {
            ConnectAddr = fmt::format("roborio-{}-frc.local", TeamNumber);
        } else {
            ConnectAddr = Hostname;
        }
    }

    PendingConnections.clear();
    ConnectionCount = 0;

    // TODO handle if connection count ever is greater then 4
    ConnectionStore[ConnectionCount] = QuicConnection{ConnectAddr, 1360};
    PendingConnections.emplace_back(&ConnectionStore[ConnectionCount]);
    ConnectionCount++;
}

void DriverStation::Impl::HandleStreamData()
{
    printf("Received Stream Data\n");
    WPI_ResetEvent(CurrentConnection->GetStreamEvent());
}

void DriverStation::Impl::HandleDatagramData()
{
    printf("Received Datagram Data\n");
    WPI_ResetEvent(CurrentConnection->GetDatagramEvent());
}

void DriverStation::Impl::HandleNewAppData()
{
    printf("Received App Data\n");
    NewDataEvent.Reset();
}

void DriverStation::Impl::HandleDisconnect()
{
    printf("Received Disconnect\n");
    CurrentConnection->Disconnect();
    CurrentConnection = nullptr;
}

DriverStation::DriverStation()
{
    pImpl = std::make_unique<Impl>();
}

DriverStation::~DriverStation() noexcept
{
}
