#pragma once

#include <functional>
#include <exception>

#include <wpi/span.h>
#include <vector>
#include <optional>

namespace tags
{
    struct TagData
    {
    public:
        TagData(wpi::span<uint8_t> FBuffer, wpi::span<uint8_t> SBuffer) noexcept : FirstBuffer{FBuffer}, SecondBuffer{SBuffer} {};

        template <typename T>
        bool Read(size_t Offset, T &Output) const noexcept
        {
            static_assert(std::is_trivial_v<T>);
            static_assert(std::is_standard_layout_v<T>);
            size_t ReadSize = Offset + sizeof(T);
            uint8_t *Buf = reinterpret_cast<uint8_t *>(&Output);

            // Copy from first buffer
            if (ReadSize <= FirstBuffer.size())
            {
                std::copy_n(FirstBuffer.data() + Offset, sizeof(T), Buf);
                return true;
            }

            if (ReadSize > Size())
            {
                return false;
            }

            // Exclusive copy from 2nd buffer
            if (Offset >= FirstBuffer.size())
            {
                // Only from second
                std::copy_n(SecondBuffer.data() + (Offset - FirstBuffer.size()), sizeof(T), Buf);
                return true;
            }

            // Copy from mix of first and 2nd
            size_t FirstCopySize = FirstBuffer.size() - Offset;
            uint8_t *MidBuffer = std::copy_n(FirstBuffer.data() + Offset, FirstCopySize, Buf);
            std::copy_n(SecondBuffer.data(), sizeof(T) - FirstCopySize, MidBuffer);

            return true;
        }

        size_t Size() const noexcept
        {
            return FirstBuffer.size() + SecondBuffer.size();
        }

        wpi::span<uint8_t> CopyTo(wpi::span<uint8_t> Buffer) const noexcept
        {
            size_t CopySize = (std::min)(Buffer.size(), Size());
            size_t FromFirstBuffer = (std::min)(CopySize, FirstBuffer.size());
            size_t FromSecondBuffer = CopySize - FromFirstBuffer;

            std::copy_n(FirstBuffer.data(), FromFirstBuffer, Buffer.data());
            std::copy_n(SecondBuffer.data(), FromSecondBuffer, Buffer.data() + FromFirstBuffer);

            return Buffer.subspan(0, CopySize);
        }

        std::optional<TagData> SubBuffer(size_t Offset, size_t Len)
        {
            if (Offset + Len <= FirstBuffer.size())
            {
                return TagData{FirstBuffer.subspan(Offset, Len), {}};
            }

            if (Size() < Offset + Len)
            {
                return {};
            }

            if (Offset >= FirstBuffer.size())
            {
                return TagData{SecondBuffer.subspan(Offset - FirstBuffer.size(), Len), {}};
            }

            return TagData{FirstBuffer.subspan(Offset), SecondBuffer.subspan(0, Len - (FirstBuffer.size() - Offset))};
        }

    private:
        wpi::span<uint8_t> FirstBuffer;
        wpi::span<uint8_t> SecondBuffer;
    };

    class TagManager
    {
    public:
        using TagFunction = std::function<void(const TagData &)>;
        using UnknownTagFunction = std::function<void(uint16_t, const TagData &)>;

        void ClearAllTagFunctions() { TagMap.clear(); }
        void AddTagFunction(uint16_t Tag, TagFunction TagFunction)
        {
            TagMap.resize(Tag + 1);
            TagMap[Tag] = TagFunction;
        }

        void SetUnknownTagHandler(UnknownTagFunction TagHandler)
        {
            if (!TagHandler)
            {
                this->UnknownTagHandler = [](uint16_t, const TagData &) {};
                return;
            }
            this->UnknownTagHandler = TagHandler;
        }

        void ReadTags(wpi::span<uint8_t> Datagram) const;
        uint32_t TryReadTag(wpi::span<uint8_t> FirstBuffer, wpi::span<uint8_t> SecondBuffer) const;

    private:
        std::vector<TagFunction> TagMap;
        UnknownTagFunction UnknownTagHandler = [](uint16_t, const TagData &) {};
    };
}
