#pragma once

#include <memory>
#include <future>

namespace ncom {
class RemoteConnection;
class Netcomm {
 public:
    Netcomm();
    ~Netcomm() noexcept;

    Netcomm(const Netcomm&) = delete;
    Netcomm(Netcomm&&) = delete;

    Netcomm& operator=(const Netcomm&) = delete;
    Netcomm& operator=(Netcomm&&) = delete;

    std::future<std::shared_ptr<RemoteConnection>> StartListener();
    struct Impl;
 private:
    std::shared_ptr<RemoteConnection> MakeRemoteConnection(void* ConnHandle);
    std::unique_ptr<Impl> pImpl;
};
}
