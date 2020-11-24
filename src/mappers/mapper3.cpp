#include <cassert>

#include "nesturbia/mappers/mapper3.hpp"

namespace nesturbia {

Mapper::ptr_t Mapper3::Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom,
                              mirror_t mirrorType) {
  // Validate PRG-ROM size
  if (prgRom.size() != 0x4000 && prgRom.size() != 0x8000) {
    return nullptr;
  }

  auto mapper = std::make_unique<Mapper3>();

  mapper->prgRom = prgRom;
  mapper->chrRom = chrRom;
  mapper->mirrorType = mirrorType;

  return mapper;
}

Mapper::mirror_t Mapper3::GetMirrorType() const { return mirrorType; }

uint8 Mapper3::ReadPRG(uint16 address) {
  assert(address >= 0x8000);
  address -= 0x8000;

  // TODO: mirroring?

  return prgRom.at(address);
}

void Mapper3::WritePRG(uint16 address, uint8 value) {
  assert(address >= 0x8000);

  // CHR bank select
  // TODO: nesdev mentions 'oversize' CHR up to 2MB (8 bits), but 32K (2 bits) is common
  chrBank = value & 0x3;
}

// TODO: can throw exception on out-of-bounds memory access due to .at()
uint8 Mapper3::ReadCHR(uint16 address) { return chrRom.at((chrBank << 13) | (address & 0x1fff)); }

void Mapper3::WriteCHR(uint16 address, uint8 value) {
  (void)address;
  (void)value;

  // TODO: temporary assert
  // assert(0);
}

} // namespace nesturbia
