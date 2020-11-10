#include <vector>

#include "nesturbia/mapper.hpp"
#include "nesturbia/mappers/mapper0.hpp"

namespace nesturbia {

Mapper::ptr_t Mapper::Create(const void *romData, size_t romDataSize) {
  if (!romData) {
    return nullptr;
  }

  const auto u8RomData = reinterpret_cast<const uint8_t *>(romData);
  std::vector<uint8> rom(u8RomData, u8RomData + romDataSize);

  if (rom.size() < 16 || rom[0] != 'N' || rom[1] != 'E' || rom[2] != 'S' || rom[3] != 0x1a) {
    return nullptr;
  }

  const auto prgRom16KUnits = rom[4];
  const auto chrRom8KUnits = rom[5];
  const auto mirrorType = static_cast<mirror_t>(rom[6] & 0x1);
  const auto mapperNumber = static_cast<uint8>((rom[6] >> 4) | (rom[7] & 0xF0));

  // TODO improve iNES header processing

  size_t currentOffset = 16;

  std::vector<uint8> prgRom;
  if (const auto prgRomSize = prgRom16KUnits * 0x4000) {
    prgRom.assign(rom.begin() + currentOffset, rom.begin() + currentOffset + prgRomSize);
    currentOffset += prgRomSize;
  }

  std::vector<uint8> chrRom;
  if (const auto chrRomSize = chrRom8KUnits * 0x2000) {
    chrRom.assign(rom.begin() + currentOffset, rom.begin() + currentOffset + chrRomSize);
  }

  switch (mapperNumber) {
  case 0:
    return Mapper0::Create(prgRom, chrRom, mirrorType);

  default:
    // Unknown or unimplemented mapper
    return nullptr;
  }
}

} // namespace nesturbia
