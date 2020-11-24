#ifndef NESTURBIA_CARTRIDGE_HPP_INCLUDED
#define NESTURBIA_CARTRIDGE_HPP_INCLUDED

#include <array>

#include "nesturbia/mapper.hpp"

namespace nesturbia {

struct Cartridge {
  // Data
  Mapper::ptr_t mapper;
  // PRG-RAM or battery-backed save RAM
  std::array<uint8, 0x2000> workRam;

  bool isNesV2 = false;
  uint8 prgRom16KUnits = 0;
  uint8 chrRom8KUnits = 0;
  // TODO: make getters for these?
  std::array<uint8_t, 16> md5Hash;
  uint32_t crc32Hash = 0;
  uint8 mapperNumber = 0;
  Mapper::mirror_t mirrorType = Mapper::mirror_t::horizontal;
  bool isBatteryBacked = false;
  bool hasTrainer = false;

  // Public functions
  bool LoadRom(const void *romData, size_t romDataSize);

  // TODO: add a getter? or just pull the public value above (workRam)?
  bool LoadBatteryBackedRAM(const void *ramData, size_t ramDataSize);

  [[nodiscard]] Mapper::mirror_t GetMirrorType() const;

  uint8 ReadPRG(uint16 address);
  void WritePRG(uint16 address, uint8 value);

  uint8 ReadCHR(uint16 address);
  void WriteCHR(uint16 address, uint8 value);

  // Private functions
};

} // namespace nesturbia

#endif // NESTURBIA_CARTRIDGE_HPP_INCLUDED
