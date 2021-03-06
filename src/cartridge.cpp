#include <cassert>
#include <vector>

#include "nesturbia/cartridge.hpp"
#include "nesturbia/mapper.hpp"
#include "nesturbia/mappers/mapper0.hpp"
#include "nesturbia/mappers/mapper1.hpp"
#include "nesturbia/mappers/mapper3.hpp"
#include "nesturbia/mappers/mapper4.hpp"
#include "nesturbia/util/crc32.hpp"
#include "nesturbia/util/md5.hpp"

namespace nesturbia {

bool Cartridge::LoadRom(const void *romData, size_t romDataSize) {
  const auto u8RomData = reinterpret_cast<const uint8_t *>(romData);
  std::vector<uint8> rom(u8RomData, u8RomData + romDataSize);

  if (rom.size() < 16 || rom[0] != 'N' || rom[1] != 'E' || rom[2] != 'S' || rom[3] != 0x1a) {
    return false;
  }

  prgRom16KUnits = rom[4];
  chrRom8KUnits = rom[5];
  mirrorType = rom[6].bit(0) ? Mapper::mirror_t::vertical : Mapper::mirror_t::horizontal;
  isBatteryBacked = rom[6].bit(1);
  hasTrainer = rom[6].bit(2);
  mapperNumber = static_cast<uint8>((rom[6] >> 4) | (rom[7] & 0xf0));
  isNesV2 = rom[6].bit(3) && !rom[6].bit(2);

  if (hasTrainer) {
    // TODO: trainers not supported for now
    return false;
  }

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

  case 3:
    mapper = Mapper3::Create(prgRom, chrRom, mirrorType);
    break;

  case 4:
    // TODO is mirrorType necessary?
    mapper = Mapper4::Create(prgRom, chrRom, mirrorType);
    break;

  default:
    // Unknown or unimplemented mapper
    printf("TODO: unsupported mapper %d\n", (int)mapperNumber);
    return false;
  }

  // Calculate ROM hashes
  crc32Hash = crc32(&rom[16], rom.size() - 16);
  md5Hash = md5(&rom[16], rom.size() - 16);

  return true;
}

bool Cartridge::LoadBatteryBackedRAM(const void *ramData, size_t ramDataSize) {
  if (!mapper || !isBatteryBacked) {
    // Cartridge not loaded or doesn't save to a battery-backed RAM
    return false;
  }

  if (!ramData || ramDataSize != workRam.size()) {
    // Invalid arguments (there must be exactly 8K of non-null data)
    return false;
  }

  memcpy(workRam.data(), ramData, ramDataSize);
  return true;
}

Mapper::mirror_t Cartridge::GetMirrorType() const {
  if (mapper) {
    return mapper->GetMirrorType();
  }

  return Mapper::mirror_t::horizontal;
}

uint8 Cartridge::ReadPRG(uint16 address) {
  if (address < 0x6000) {
    // TODO support expansion ROM
    assert(0);
    return 0;
  }

  if (address < 0x8000) {
    return workRam[address - 0x6000];
  }

  if (mapper) {
    return mapper->ReadPRG(address);
  }

  return 0;
}

void Cartridge::WritePRG(uint16 address, uint8 value) {
  if (address < 0x6000) {
    // TODO support expansion ROM
    assert(0);
    return;
  }

  if (address < 0x8000) {
    workRam[address - 0x6000] = value;
    return;
  }

  if (mapper) {
    mapper->WritePRG(address, value);
  }
}

uint8 Cartridge::ReadCHR(uint16 address) {
  if (mapper) {
    return mapper->ReadCHR(address);
  }

  return 0;
}

void Cartridge::WriteCHR(uint16 address, uint8 value) {
  if (mapper) {
    mapper->WriteCHR(address, value);
  }
}

} // namespace nesturbia
