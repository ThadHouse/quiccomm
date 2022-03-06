#pragma once

#include <memory>
#include "wpi/Synchronization.h"
#include <vector>
#include <wpi/span.h>
#include <string>

namespace qapi
{
class QuicConnection {
public:

    QuicConnection() noexcept;
    explicit QuicConnection(uint16_t Port);
    QuicConnection(std::string Host, uint16_t Port);

    ~QuicConnection() noexcept;

    QuicConnection(const QuicConnection&) = delete;
    QuicConnection(QuicConnection&&) noexcept;

    QuicConnection& operator=(const QuicConnection&) = delete;
    QuicConnection& operator=(QuicConnection&&) noexcept;

    WPI_EventHandle GetReadyEvent() noexcept {
        return ReadyEvent.GetHandle();
    }
    WPI_EventHandle GetDatagramEvent() noexcept {
        return DatagramEvent.GetHandle();
    }
    WPI_EventHandle GetStreamEvent() noexcept {
        return StreamEvent.GetHandle();
    }
    WPI_EventHandle GetDisconnectedEvent() noexcept {
        return DisconnectedEvent.GetHandle();
    }

    std::vector<uint8_t> GetStreamData() noexcept;
    std::vector<std::unique_ptr<uint8_t[]>> GetDatagramData() noexcept;

    void WriteDatagram(wpi::span<uint8_t> datagram);
    void WriteStream(wpi::span<uint8_t> data);

    void* GetStreamHandle() noexcept;
    void* GetConnectionHandle() noexcept;

    // Only valid on client
    void Disconnect();

    struct Impl;
private:
    wpi::Event ReadyEvent;
    wpi::Event DatagramEvent{true};
    wpi::Event StreamEvent{true};
    wpi::Event DisconnectedEvent;
    std::unique_ptr<Impl> pImpl;
};
} // namespace qapi
