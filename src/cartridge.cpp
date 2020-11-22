// TODO: temporary for MD5/CRC32 below
#include <iostream>
#include <vector>

#include "nesturbia/cartridge.hpp"
#include "nesturbia/mapper.hpp"
#include "nesturbia/mappers/mapper0.hpp"
#include "nesturbia/mappers/mapper1.hpp"
#include "nesturbia/util/crc32.hpp"
#include "nesturbia/util/md5.hpp"

namespace nesturbia {

bool Cartridge::LoadRom(const void *romData, size_t romDataSize) {
  const auto u8RomData = reinterpret_cast<const uint8_t *>(romData);
  std::vector<uint8> rom(u8RomData, u8RomData + romDataSize);

  if (rom.size() < 16 || rom[0] != 'N' || rom[1] != 'E' || rom[2] != 'S' || rom[3] != 0x1a) {
    return false;
  }

  const auto prgRom16KUnits = rom[4];
  const auto chrRom8KUnits = rom[5];
  const auto mirrorType = rom[6].bit(0) ? Mapper::mirror_t::vertical : Mapper::mirror_t::horizontal;
  const auto mapperNumber = static_cast<uint8>((rom[6] >> 4) | (rom[7] & 0xF0));

  // TODO improve iNES header processing

  // Get the size of the PRG-ROM and CHR-ROM sections in bytes
  const auto prgRomSize = prgRom16KUnits * 0x4000;
  const auto chrRomSize = chrRom8KUnits * 0x2000;

  // Calculate the expected size of the ROM file (including header)
  const size_t expectedRomSize = 16 + prgRomSize + chrRomSize;
  if (romDataSize != expectedRomSize) {
    // The ROM file has an unexpected length
    return false;
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
    mapper = Mapper0::Create(prgRom, chrRom, mirrorType);
    break;

  case 1:
    // TODO: does mirror type matter for MMC1?
    mapper = Mapper1::Create(prgRom, chrRom);
    break;

  default:
    // Unknown or unimplemented mapper
    printf("TODO: unsupported mapper %d\n", (int)mapperNumber);
    return false;
  }

  // TODO: temporary testing
  std::cout << "MD5: " << md5(u8RomData + 16, romDataSize - 16) << std::endl;
  std::cout << "CRC-32: " << crc32(u8RomData + 16, romDataSize - 16) << std::endl;

  return true;
}

Mapper::mirror_t Cartridge::GetMirrorType() const {
  if (mapper) {
    return mapper->GetMirrorType();
  }

  return Mapper::mirror_t::horizontal;
}

uint8 Cartridge::Read(uint16 address) {
  if (mapper) {
    return mapper->Read(address);
  }

  return 0;
}

void Cartridge::Write(uint16 address, uint8 value) {
  if (mapper) {
    mapper->Write(address, value);
  }
}

uint8 Cartridge::ReadChr(uint16 address) {
  if (mapper) {
    return mapper->ReadChr(address);
  }

  return 0;
}

void Cartridge::WriteChr(uint16 address, uint8 value) {
  if (mapper) {
    mapper->WriteChr(address, value);
  }
}

} // namespace nesturbia
