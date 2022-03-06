#pragma once

#include <memory>
#include <wpi/Synchronization.h>
#include <vector>
#include <wpi/mutex.h>

typedef enum DS_EventType
{
    DS_Event_Disconnected,
    DS_Event_Connected
} DS_EventType;

typedef struct DS_Event
{
    int32_t Type;
} DS_Event;

namespace ds
{

    class DsEvent : public DS_Event
    {
    public:
        static DsEvent DisconnectedEvent() noexcept;
        static DsEvent ConnectedEvent() noexcept;

    private:
        explicit DsEvent(DS_EventType EventType)
        {
            Type = EventType;
        }
    };

    static_assert(sizeof(DsEvent) == sizeof(DS_Event));

    class DriverStation
    {
    public:
        DriverStation();
        ~DriverStation() noexcept;

        void SetTeamNumber(int teamNumber);
        void SetHostname(std::string host);

        void AckDisconnect();

        void SendControlPacket();

        void SendGameData();

        WPI_EventHandle GetEventHandle()
        {
            return Event.GetHandle();
        }

        void GetEvents(std::vector<DS_Event> &CachedEvents)
        {
            std::scoped_lock Lock{EventMutex};
            Event.Reset();
            Events.swap(CachedEvents);
            Events.clear();
        }

    private:
        struct Impl;
        wpi::Event Event{true};
        std::vector<DS_Event> Events;
        wpi::mutex EventMutex;
        std::unique_ptr<Impl> pImpl;
    };
}
