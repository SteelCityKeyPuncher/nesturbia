#include <cassert>

#include "nesturbia/mappers/mapper0.hpp"

namespace nesturbia {

Mapper::ptr_t Mapper0::Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom,
                              mirror_t mirrorType) {
  // Validate PRG-ROM size
  if (prgRom.size() != 0x4000 && prgRom.size() != 0x8000) {
    return nullptr;
  }

  auto mapper = std::make_unique<Mapper0>();

  mapper->prgRom = prgRom;
  mapper->chrRom = chrRom;
  mapper->mirrorType = mirrorType;

  return mapper;
}

Mapper::mirror_t Mapper0::GetMirrorType() const { return mirrorType; }

uint8 Mapper0::ReadPRG(uint16 address) {
  assert(address >= 0x8000);
  address -= 0x8000;

  // Mirror if using 16K PRG-ROM
  // TODO this could be more efficient
  if (prgRom.size() == 0x4000) {
    address &= 0x3fff;
  }

  return prgRom[address];
}

void Mapper0::WritePRG(uint16 address, uint8) {
  assert(address >= 0x8000);
  // TODO: temporary test
  assert(0);
}

uint8 Mapper0::ReadCHR(uint16 address) {
  if (address < chrRom.size()) {
    return chrRom[address];
  }

  // TODO: could have CHR-RAM
  assert(0);
  return 0;
}

void Mapper0::WriteCHR(uint16 address, uint8 value) {
  if (address < chrRom.size()) {
    return;
  }

  (void)value;
  // TODO temporary assert
  assert(0);
}

} // namespace nesturbia
