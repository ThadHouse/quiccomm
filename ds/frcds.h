#pragma once

#include <memory>
#include <wpi/Synchronization.h>
#include <vector>

namespace ds {
struct DSEvent;

class DriverStation {
public:
    DriverStation();
    ~DriverStation() noexcept;

    void SetTeamNumber(int teamNumber);
    void SetHostname(std::string host);

    WPI_EventHandle GetEventHandle();
    std::vector<std::unique_ptr<DSEvent>> GetEvents();

private:
struct Impl;
std::unique_ptr<Impl> pImpl;
};
}
