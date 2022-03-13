#pragma once

#include <stdint.h>
#include <functional>
#include <wpi/span.h>

namespace qapi
{
    struct DataBuffer
    {
        uint32_t Length;
        uint8_t *Buffer;
    };

    struct Callbacks
    {
        using ReadyFunction = std::function<void()>;
        using DisconnectedFunction = std::function<void()>;
        using DatagramReceiveFunction = std::function<void(const DataBuffer &)>;
        using StreamReceiveFunction = std::function<void(wpi::span<const DataBuffer>)>;

        ReadyFunction Ready;
        DisconnectedFunction Disconnected;
        DatagramReceiveFunction DatagramReceived;
        StreamReceiveFunction StreamReceived;
        StreamReceiveFunction ControlStreamReceived;
    };
}
