#include "netcomm.h"
#include "QuicConnection.h"
#include "QuicApi.h"
#include <thread>
#include <wpi/mutex.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <wpi/SmallVector.h>

using namespace ncom;
using namespace qapi;

struct Netcomm::Impl
{
    Netcomm *Owner;
    QuicApi Api{"Netcomm"};
    QuicConnection Connection{1360};

    std::thread EventThread;
    std::atomic_bool ThreadRunning{true};
    wpi::Event EndThreadEvent;
    wpi::Event NewDataEvent{true};

    wpi::mutex AppDataMutex;

    void ThreadRun();
    void HandleNewAppData();
    void HandleReadyEvent();
    void HandleStreamData();
    void HandleDatagramData();
    void HandleDisconnect();

    Impl(Netcomm *Owner) : Owner{Owner}, EventThread{[&]
                                                     { ThreadRun(); }}
    {
    }

    ~Impl()
    {
        ThreadRunning = false;
        EndThreadEvent.Set();
        if (EventThread.joinable())
        {
            EventThread.join();
        }
    }
};

void Netcomm::Impl::ThreadRun()
{
    std::array<WPI_Handle, 6> Events{
        EndThreadEvent.GetHandle(),
        NewDataEvent.GetHandle(),
        Connection.GetDisconnectedEvent(),
        Connection.GetReadyEvent(),
        Connection.GetDatagramEvent(),
        Connection.GetStreamEvent()};

    std::array<WPI_Handle, 6> SignaledEventsStorage;
    while (ThreadRunning)
    {
        const auto SignaledEvents = wpi::WaitForObjects(Events, SignaledEventsStorage);

        for (auto &&i : SignaledEvents)
        {
            if (i == Events[0])
            {
                break;
            }
            else if (i == Events[1])
            {
                HandleNewAppData();
            }
            else if (i == Events[2])
            {
                HandleDisconnect();
            }
            else if (i == Events[3])
            {
                HandleReadyEvent();
            }
            else if (i == Events[4])
            {
                HandleDatagramData();
            }
            else if (i == Events[5])
            {
                HandleStreamData();
            }
        }
    }
}

void Netcomm::Impl::HandleNewAppData()
{
}

void Netcomm::Impl::HandleReadyEvent()
{
    std::scoped_lock Lock{Owner->EventMutex};
    Owner->Events.emplace_back(NetcommEvent::ConnectedEvent());
    Owner->Event.Set();
}

void Netcomm::Impl::HandleStreamData()
{
    std::vector<uint8_t> StreamData;
    Connection.GetStreamData(StreamData);
    printf("Handled stream %d %d\n", (int)StreamData.size(), StreamData[0]);
}

void Netcomm::Impl::HandleDatagramData()
{
    std::vector<DatagramBuffer> Datagrams;
    Connection.GetDatagramData(Datagrams);
    for (auto&& dg : Datagrams) {
        printf("Handled datagram %d %d\n", dg.Length, dg.Buffer[0]);
    }
}

void Netcomm::Impl::HandleDisconnect()
{
    std::scoped_lock Lock{Owner->EventMutex};
    Owner->Events.emplace_back(NetcommEvent::DisconnectedEvent());
    Owner->Event.Set();
}

Netcomm::Netcomm()
{
    pImpl = std::make_unique<Impl>(this);
}

Netcomm::~Netcomm() noexcept
{
}

NetcommEvent NetcommEvent::ConnectedEvent() noexcept
{
    return NetcommEvent{NETCOMM_Event_Connected};
}

NetcommEvent NetcommEvent::DisconnectedEvent() noexcept
{
    return NetcommEvent{NETCOMM_Event_Disconnected};
}
