#include "dsevents.h"

using namespace ds;

DsEvents::DsEvents(const DS_EventCallbacks *EventCallbacks) noexcept
{
    DsEventCallbacks = *EventCallbacks;
}

uint32_t DsEvents::ReadEvents(DS_Event *Events, uint32_t EventCount, uint32_t *RemainingEvents) noexcept
{
    uint32_t ReadCount = 0;
    std::scoped_lock Lock{DsEventMutex};
    if (OverflowDsEvents.empty())
    {
        while (StaticDsEvents.size() != 0 && ReadCount < EventCount)
        {
            Events[ReadCount] = StaticDsEvents.pop_front();
            ReadCount++;
        }
        *RemainingEvents = StaticDsEvents.size();
        return ReadCount;
    }

    while (StaticDsEvents.size() != 0 && ReadCount < EventCount)
    {
        Events[ReadCount] = StaticDsEvents.pop_front();
        ReadCount++;
        if (!OverflowDsEvents.empty())
        {
            StaticDsEvents.emplace_front(OverflowDsEvents.front());
            OverflowDsEvents.pop();
        }
    }
    *RemainingEvents = StaticDsEvents.size() + OverflowDsEvents.size();
    return ReadCount;
}

void DsEvents::DatagramReceive(const qapi::DataBuffer &Datagram) noexcept
{
    (void)Datagram;
}

void DsEvents::StreamReceive(std::span<const qapi::DataBuffer> Buffers) noexcept
{
    (void)Buffers;
}
