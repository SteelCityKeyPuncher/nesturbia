#ifndef UTIL_CRC32_HPP_INCLUDED
#define UTIL_CRC32_HPP_INCLUDED

#include <cstdint>
#include <string>

namespace nesturbia {

static inline std::string crc32(const void *message, size_t length) {
  uint32_t crc = 0xffffffff;

  for (size_t i = 0; i < length; i++) {
    auto crcLookup = (crc ^ reinterpret_cast<const uint8_t *>(message)[i]) & 0xff;
    for (int j = 0; j < 8; j++) {
      crcLookup = (crcLookup >> 1) ^ (crcLookup & 0x1 ? 0xedb88320 : 0x0);
    }

    crc = (crc >> 8) ^ crcLookup;
  }

  std::string result = "0x";
  for (int i = 0; i < 8; i++) {
    result += "0123456789abcdef"[(crc >> 28 & 0xf) ^ 0xf];
    crc <<= 4;
  }

  return result;
}

} // namespace nesturbia

#endif // UTIL_CRC32_HPP_INCLUDED
