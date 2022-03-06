#pragma once

#include <memory>
#include <wpi/Synchronization.h>
#include <vector>
#include <wpi/mutex.h>

typedef enum NETCOMM_EventType
{
    NETCOMM_Event_Disconnected,
    NETCOMM_Event_Connected
} NETCOMM_EventType;

typedef struct NETCOMM_Event
{
    int32_t Type;
} NETCOMM_Event;

namespace ncom
{
    class NetcommEvent : public NETCOMM_Event
    {
    public:
        static NetcommEvent DisconnectedEvent() noexcept;
        static NetcommEvent ConnectedEvent() noexcept;

    private:
        explicit NetcommEvent(NETCOMM_EventType EventType)
        {
            Type = EventType;
        }
    };

    static_assert(sizeof(NetcommEvent) == sizeof(NETCOMM_Event));

    class Netcomm
    {
    public:
        Netcomm();
        ~Netcomm() noexcept;

        WPI_EventHandle GetEventHandle()
        {
            return Event.GetHandle();
        }

        void GetEvents(std::vector<NETCOMM_Event> &CachedEvents)
        {
            std::scoped_lock Lock{EventMutex};
            Event.Reset();
            Events.swap(CachedEvents);
            Events.clear();
        }

        struct Impl;

    private:
        wpi::Event Event{true};
        std::vector<NETCOMM_Event> Events;
        wpi::mutex EventMutex;
        std::unique_ptr<Impl> pImpl;
    };
}
