#pragma once

#include <memory>
#include <future>

namespace ncom {
class Netcomm;
class RemoteConnection {
private:
    struct private_init {};

public:
    friend class Netcomm;
    RemoteConnection(const private_init&, void* ConnHandle);
    ~RemoteConnection() noexcept;

    RemoteConnection(const RemoteConnection&) = delete;
    RemoteConnection(RemoteConnection&&) = delete;

    RemoteConnection& operator=(const RemoteConnection&) = delete;
    RemoteConnection& operator=(RemoteConnection&&) = delete;

    std::future<void> GetStreamReadyFuture();

    struct Impl;
private:
    std::unique_ptr<Impl> pImpl;

};
}
