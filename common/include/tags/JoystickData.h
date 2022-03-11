#pragma once

#include "TagManager.h"
#include "CommonData.h"
#include "util/BufferHelpers.h"

namespace tags {
class JoystickData {
    public:
        constexpr static uint16_t TagNumber = 1;
        explicit JoystickData(const TagData& Data) noexcept;
        explicit JoystickData(const COMM_JoystickData& Data) noexcept : StickData{Data} {
        }

        void WriteTag(util::OStreamWriter Buffer) const;

        const COMM_JoystickData& GetStickData() const {
            return StickData;
        }

    private:
        COMM_JoystickData StickData;
};
}
