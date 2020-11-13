#include <cassert>

#include "nesturbia/mappers/mapper0.hpp"

namespace nesturbia {

Mapper::ptr_t Mapper0::Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom,
                              mirror_t mirrorType) {
  // Validate PRG-ROM size
  if (prgRom.size() != 0x4000 && prgRom.size() != 0x8000) {
    return nullptr;
  }

  // Validate CHR-ROM size
  if (chrRom.size() != 0x2000) {
    return nullptr;
  }

  auto mapper = std::make_unique<Mapper0>();

  mapper->prgRom = prgRom;
  mapper->chrRom = chrRom;
  mapper->mirrorType = mirrorType;

  return mapper;
}

uint8 Mapper0::Read(uint16 address) {
  if (address < 0x8000) {
    // TODO check this
    return 0;
  }

  address -= 0x8000;

  // Mirror if using 16K PRG-ROM
  // TODO this could be more efficient
  if (prgRom.size() == 0x4000) {
    address &= 0x3fff;
  }

  return prgRom[address];
}

void Mapper0::Write(uint16, uint8) {}

uint8 Mapper0::ReadChr(uint16 address) {
  assert(address < chrRom.size());
  return chrRom[address];
}

void Mapper0::WriteChr(uint16, uint8) {
  // TODO I think NROM is read-only?
}

} // namespace nesturbia
