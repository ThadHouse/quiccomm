#pragma once

#include "driverstation.h"
#include "dsevents.h"
#include <memory>

namespace ds
{
    class RobotComms {
        public:
        explicit RobotComms(ds::DsEvents* DsEvents, DS_Status* Status) noexcept;
        ~RobotComms() noexcept;

        bool SetTeam(const char* Team) noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };
} // namespace ds
