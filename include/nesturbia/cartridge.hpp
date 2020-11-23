#ifndef NESTURBIA_CARTRIDGE_HPP_INCLUDED
#define NESTURBIA_CARTRIDGE_HPP_INCLUDED

#include <array>

#include "nesturbia/mapper.hpp"

namespace nesturbia {

struct Cartridge {
  // Data
  Mapper::ptr_t mapper;
  // TODO: make getters for these?
  std::array<uint8_t, 16> md5Hash;
  uint32_t crc32Hash = 0;

  // Public functions
  bool LoadRom(const void *romData, size_t romDataSize);

  [[nodiscard]] Mapper::mirror_t GetMirrorType() const;

  uint8 Read(uint16 address);
  void Write(uint16 address, uint8 value);

  uint8 ReadChr(uint16 address);
  void WriteChr(uint16 address, uint8 value);

  // Private functions
};

} // namespace nesturbia

#endif // NESTURBIA_CARTRIDGE_HPP_INCLUDED
