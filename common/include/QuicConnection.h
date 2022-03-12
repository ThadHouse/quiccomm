#pragma once

#include <memory>
#include <wpi/span.h>
#include <string>
#include <functional>

namespace qapi
{
    class QuicConnection
    {
    public:
        struct DataBuffer {
            uint32_t Length;
            uint8_t* Buffer;
        };

        struct Callbacks {
            using ReadyFunction = std::function<void()>;
            using DisconnectedFunction = std::function<void()>;
            using DatagramReceiveFunction = std::function<void(const DataBuffer&)>;
            using StreamReceiveFunction = std::function<void(wpi::span<const DataBuffer>)>;

            ReadyFunction Ready;
            DisconnectedFunction Disconnected;
            DatagramReceiveFunction DatagramReceived;
            StreamReceiveFunction StreamReceived;
            StreamReceiveFunction ControlStreamReceived;
        };

        QuicConnection();
        QuicConnection(uint16_t Port, Callbacks Cbs);
        QuicConnection(std::string Host, uint16_t Port, Callbacks Cbs);

        ~QuicConnection() noexcept;

        QuicConnection(const QuicConnection &) = delete;

        QuicConnection &operator=(const QuicConnection &) = delete;

        void WriteDatagram(wpi::span<uint8_t> datagram);
        void WriteStream(wpi::span<uint8_t> data);
        void WriteControlStream(wpi::span<uint8_t> data);

        void *GetStreamHandle() noexcept;
        void *GetControlStreamHandle() noexcept;
        void *GetConnectionHandle() noexcept;

        // Only valid on client
        void Disconnect();

        struct Impl;

    private:
        std::unique_ptr<Impl> pImpl;
    };
} // namespace qapi
