#pragma once

#include <memory>
#include "wpi/Synchronization.h"
#include <vector>
#include <wpi/span.h>
#include <string>
#include <wpi/mutex.h>

namespace qapi
{
    struct DatagramBuffer
    {
        std::unique_ptr<uint8_t[]> Buffer;
        uint64_t Timestamp;
        uint32_t Length;
    };

    class QuicConnection
    {
    public:
        QuicConnection() noexcept;
        explicit QuicConnection(uint16_t Port);
        QuicConnection(std::string Host, uint16_t Port);

        ~QuicConnection() noexcept;

        QuicConnection(const QuicConnection &) = delete;

        QuicConnection &operator=(const QuicConnection &) = delete;

        WPI_EventHandle GetReadyEvent() noexcept
        {
            return ReadyEvent.GetHandle();
        }
        WPI_EventHandle GetDatagramEvent() noexcept
        {
            return DatagramEvent.GetHandle();
        }
        WPI_EventHandle GetStreamEvent() noexcept
        {
            return StreamEvent.GetHandle();
        }
        WPI_EventHandle GetDisconnectedEvent() noexcept
        {
            return DisconnectedEvent.GetHandle();
        }

        void GetStreamData(std::vector<uint8_t> &CachedData) noexcept
        {
            std::scoped_lock Lock{StreamMutex};
            StreamEvent.Reset();
            StreamData.swap(CachedData);
            StreamData.clear();
        }

        void GetDatagramData(std::vector<DatagramBuffer> &CachedData) noexcept
        {
            std::scoped_lock Lock{DatagramMutex};
            DatagramEvent.Reset();
            DatagramData.swap(CachedData);
            DatagramData.clear();
        }

        void WriteDatagram(wpi::span<uint8_t> datagram);
        void WriteStream(wpi::span<uint8_t> data);

        void *GetStreamHandle() noexcept;
        void *GetConnectionHandle() noexcept;

        // Only valid on client
        void Disconnect();

        struct Impl;

    private:
        wpi::Event ReadyEvent;
        wpi::Event DatagramEvent{true};
        wpi::Event StreamEvent{true};
        wpi::Event DisconnectedEvent;
        wpi::mutex DatagramMutex;
        wpi::mutex StreamMutex;
        std::unique_ptr<Impl> pImpl;
        std::vector<uint8_t> StreamData;
        std::vector<DatagramBuffer> DatagramData;
    };
} // namespace qapi
