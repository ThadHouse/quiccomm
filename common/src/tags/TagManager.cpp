#include "tags/TagManager.h"

using namespace tags;

#define BSwap(x) ((uint16_t)((((x)&0x00ff) << 8) | (((x)&0xff00) >> 8)))

void TagManager::ReadTags(std::span<uint8_t> Datagram) const
{

    uint32_t ReadCount = 0;
    while ((ReadCount = TryReadTag(Datagram, {})) != 0)
    {
        Datagram = Datagram.subspan(ReadCount);
    }
}

uint32_t TagManager::TryReadTag(std::span<uint8_t> FirstBuffer, std::span<uint8_t> SecondBuffer) const
{
    TagData ReadBuffer{FirstBuffer, SecondBuffer};
    uint16_t TagSize;
    if (!ReadBuffer.Read(0, TagSize))
    {
        return 0;
    }

    TagSize = BSwap(TagSize);

    std::optional<TagData> TagBuffer = ReadBuffer.SubBuffer(2, TagSize - 2);
    if (!TagBuffer.has_value())
    {
        return 0;
    }

    uint16_t Tag;
    TagBuffer.value().Read(0, Tag);
    Tag = BSwap(Tag);

    if (TagMap.size() > Tag)
    {
        TagMap[Tag](TagBuffer.value());
    }
    else
    {
        UnknownTagHandler(Tag, TagBuffer.value());
    }

    return TagSize;
}
