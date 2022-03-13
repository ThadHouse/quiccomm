#pragma once

#include <memory>
#include <functional>

namespace ds {
class ControlPacketThread {
    public:
        using Callback = std::function<void()>;

        explicit ControlPacketThread(Callback Cb, int32_t* Status) noexcept;
        ~ControlPacketThread() noexcept;
    private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
}
