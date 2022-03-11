#pragma once

#ifndef _MSC_VER
#pragma GCC diagnostic ignored "-Wunused-parameter"
#else
#pragma warning(disable:4458)
#endif

#include <wpi/raw_ostream.h>

#include "util/SequenceNumber.h"

namespace util {

struct OStreamWriter {
  wpi::raw_ostream& stream;

  OStreamWriter(wpi::raw_ostream& buf)  // NOLINT (runtime/explicit)
      : stream{buf} {}

  OStreamWriter WriteU8(uint8_t val) {
    stream << val;
    return *this;
  }

  OStreamWriter WriteS8(int8_t val) {
    stream << val;
    return *this;
  }

  OStreamWriter WriteU16(uint16_t val) {
    stream << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteS16(int16_t inVal) {
    uint16_t val = static_cast<uint16_t>(inVal);
    stream << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteU32(uint32_t val) {
    stream << static_cast<uint8_t>(val >> 24) << static_cast<uint8_t>(val >> 16)
           << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteS32(int32_t inVal) {
    uint32_t val = static_cast<uint32_t>(inVal);
    stream << static_cast<uint8_t>(val >> 24) << static_cast<uint8_t>(val >> 16)
           << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteU64(uint64_t val) {
    stream << static_cast<uint8_t>(val >> 56) << static_cast<uint8_t>(val >> 48)
           << static_cast<uint8_t>(val >> 40) << static_cast<uint8_t>(val >> 32)
           << static_cast<uint8_t>(val >> 24) << static_cast<uint8_t>(val >> 16)
           << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteS64(int64_t inVal) {
    uint64_t val = static_cast<uint64_t>(inVal);
    stream << static_cast<uint8_t>(val >> 56) << static_cast<uint8_t>(val >> 48)
           << static_cast<uint8_t>(val >> 40) << static_cast<uint8_t>(val >> 32)
           << static_cast<uint8_t>(val >> 24) << static_cast<uint8_t>(val >> 16)
           << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteFloat(float fVal) {
    uint32_t val = wpi::FloatToBits(fVal);
    stream << static_cast<uint8_t>(val >> 24) << static_cast<uint8_t>(val >> 16)
           << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteDouble(double dVal) {
    uint64_t val = wpi::DoubleToBits(dVal);
    stream << static_cast<uint8_t>(val >> 56) << static_cast<uint8_t>(val >> 48)
           << static_cast<uint8_t>(val >> 40) << static_cast<uint8_t>(val >> 32)
           << static_cast<uint8_t>(val >> 24) << static_cast<uint8_t>(val >> 16)
           << static_cast<uint8_t>(val >> 8) << static_cast<uint8_t>(val);
    return *this;
  }

  OStreamWriter WriteSequenceNumber(util::SequenceNumber val) {
    return WriteU16((uint16_t)val.value());
  }

  OStreamWriter WriteArray(const uint8_t* data, size_t size) {
    stream.write(data, size);
    return *this;
  }
};

}  // namespace util
