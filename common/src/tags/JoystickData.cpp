#include "tags/JoystickData.h"

using namespace tags;

JoystickData::JoystickData(const TagData &Data) noexcept
{
    std::memset(&StickData, 0, sizeof(StickData));
    Data.Read(2, StickData.joystickNumber);
    Data.Read(3, StickData.axesCount);
    Data.Read(4, StickData.povsCount);
    Data.Read(5, StickData.buttonsCount);

    if (StickData.axesCount > COMM_MAX_JOYSTICK_AXES ||
        StickData.buttonsCount > COMM_MAX_JOYSTICK_BUTTONS ||
        StickData.povsCount > COMM_MAX_JOYSTICK_POVS ||
        StickData.joystickNumber > COMM_MAX_JOYSTICKS)
    {
        return;
    }

    if (Data.Size() < 6 + (StickData.axesCount * 2) + (StickData.povsCount * 2) + 8)
    {
        return;
    }

    Data.Read(6, StickData.buttons);

    uint16_t StickCount = StickData.axesCount;

    for (int i = 0; i < StickCount; i++)
    {
        int16_t AxisValue;
        Data.Read(8 + (i * 2), AxisValue);
        StickData.axes[i] = (float)AxisValue / 0x8FFF;
    }

    for (int i = 0; i < StickData.povsCount; i++)
    {
        int16_t POVValue;
        Data.Read(8 + StickCount + (i * 2), POVValue);
        StickData.povs[i] = POVValue;
    }

    StickData.isConnected = true;
}

void JoystickData::WriteTag(util::OStreamWriter Buffer) const {
    if (!StickData.isConnected) {
        return;
    }

    if (StickData.axesCount > COMM_MAX_JOYSTICK_AXES ||
        StickData.buttonsCount > COMM_MAX_JOYSTICK_BUTTONS ||
        StickData.povsCount > COMM_MAX_JOYSTICK_POVS ||
        StickData.joystickNumber > COMM_MAX_JOYSTICKS)
    {
        return;
    }

    // 2 Size + 2 Tag + 3 Length + 1 StickNum + 8 Buttons
    uint16_t TotalLength = 16 + (2 * StickData.axesCount) + (2 * StickData.povsCount);

    Buffer.WriteU16(TotalLength);
    Buffer.WriteU16(TagNumber);
    Buffer.WriteU8(StickData.joystickNumber);
    Buffer.WriteU8(StickData.axesCount);
    Buffer.WriteU8(StickData.povsCount);
    Buffer.WriteU8(StickData.buttonsCount);
    Buffer.WriteU64(StickData.buttons);

    for (int i = 0; i < StickData.axesCount; i++) {
        Buffer.WriteS16((int16_t)(StickData.axes[i] * 0x8FFF));
    }


    for (int i = 0; i < StickData.povsCount; i++) {
        Buffer.WriteS16(StickData.povs[i]);
    }
}
