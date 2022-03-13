#pragma once

#include <memory>
#include <wpi/span.h>
#include <string>
#include <functional>
#include "QuicCallbacks.h"

namespace qapi
{
    class QuicConnection
    {
    public:

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
