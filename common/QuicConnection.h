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

    explicit QuicConnection(uint16_t Port);
    QuicConnection(std::string Host, uint16_t Port);

    ~QuicConnection() noexcept;

    QuicConnection(const QuicConnection&) = delete;
    QuicConnection(QuicConnection&&) = delete;

    QuicConnection& operator=(const QuicConnection&) = delete;
    QuicConnection& operator=(QuicConnection&&) = delete;

    WPI_EventHandle GetReadyEvent() noexcept;
    WPI_EventHandle GetDatagramEvent() noexcept;
    WPI_EventHandle GetStreamEvent() noexcept;
    WPI_EventHandle GetDisconnectedEvent() noexcept;

    std::vector<uint8_t> GetStreamData() noexcept;
    std::vector<std::unique_ptr<uint8_t[]>> GetDatagramData() noexcept;

    void WriteDatagram(wpi::span<uint8_t> datagram);
    void WriteStream(wpi::span<uint8_t> data);

    void* GetStreamHandle() noexcept;
    void* GetConnectionHandle() noexcept;

    struct Impl;
private:

    std::unique_ptr<Impl> pImpl;
};
} // namespace qapi
