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
  if (mapper->chrRom.empty()) {
    // Empty CHR-ROM means that we get 8K CHR-RAM
    mapper->chrRam.resize(0x2000);
  }

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

// TODO: can throw exception on out-of-bounds memory access due to .at()
uint8 Mapper0::ReadCHR(uint16 address) {
  if (!chrRom.empty()) {
    return chrRom.at(address);
  }

  if (!chrRam.empty()) {
    return chrRam.at(address);
  }

  assert(0);
  return 0;
}

void Mapper0::WriteCHR(uint16 address, uint8 value) {
  if (address < chrRam.size()) {
    chrRam[address] = value;
    return;
  }

  // TODO: temporary assert
  assert(0);
}

} // namespace nesturbia
