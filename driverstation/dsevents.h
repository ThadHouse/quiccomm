#pragma once

#include "driverstation.h"
#include "wpi/mutex.h"
#include "wpi/static_circular_buffer.h"
#include <queue>
#include "QuicCallbacks.h"

namespace ds
{
    class DsEvents
    {
    public:
        explicit DsEvents(const DS_EventCallbacks *EventCallbacks) noexcept;
        uint32_t ReadEvents(DS_Event *Events, uint32_t EventCount, uint32_t *RemainingEvents) noexcept;

        qapi::Callbacks GetQuicCallbacks() noexcept {
            return {
                nullptr,
                nullptr,
                [&](const auto& a){DatagramReceive(a);},
                [&](auto a){StreamReceive(a);},
                [&](auto a){StreamReceive(a);},
            };
        }

    private:
        void DatagramReceive(const qapi::DataBuffer& Datagram) noexcept;
        void StreamReceive(wpi::span<const qapi::DataBuffer> Buffers) noexcept;

        DS_EventCallbacks DsEventCallbacks;
        wpi::mutex DsEventMutex;
        wpi::static_circular_buffer<DS_Event, 64> StaticDsEvents;
        std::queue<DS_Event> OverflowDsEvents;
    };
}
