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

  // Get the size of the PRG-ROM and CHR-ROM sections in bytes
  const auto prgRomSize = prgRom16KUnits * 0x4000;
  const auto chrRomSize = chrRom8KUnits * 0x2000;

  // Calculate the expected size of the ROM file (including header)
  const size_t expectedRomSize = 16 + prgRomSize + chrRomSize;
  if (romDataSize != expectedRomSize) {
    // The ROM file has an unexpected length
    return nullptr;
  }

  std::vector<uint8> prgRom;
  if (prgRomSize != 0) {
    prgRom.assign(rom.begin() + 16, rom.begin() + 16 + prgRomSize);
  }

  std::vector<uint8> chrRom;
  if (chrRomSize != 0) {
    const auto startOffset = 16 + prgRom.size();
    chrRom.assign(rom.begin() + startOffset, rom.begin() + startOffset + chrRomSize);
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
