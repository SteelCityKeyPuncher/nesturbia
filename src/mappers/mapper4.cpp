#include <cassert>

#include "nesturbia/mappers/mapper4.hpp"

namespace nesturbia {

// TODO: this does nothing useful for now; nes-test-roms/stomper/smwstomp.nes uses this mapper
Mapper::ptr_t Mapper4::Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom,
                              mirror_t mirrorType) {
  // TODO: Validate PRG-ROM size?

  auto mapper = std::make_unique<Mapper4>();

  // TODO: implement
  (void)prgRom;
  (void)chrRom;

  mapper->mirrorType = mirrorType;

  return mapper;
}

Mapper::mirror_t Mapper4::GetMirrorType() const { return mirrorType; }

uint8 Mapper4::ReadPRG(uint16 address) {
  assert(address >= 0x8000);
  address -= 0x8000;

  // TODO temporary assert
  assert(0);
  return 0;
}

void Mapper4::WritePRG(uint16 address, uint8) {
  assert(address >= 0x8000);

  if (address < 0xa000) {
    if (address.bit(0)) {
      // Odd: bank data
      // TODO
    } else {
      // Even: bank select
      // TODO
    }
  } else if (address < 0xc000) {
    if (address.bit(0)) {
      // Odd: PRG-RAM protect
      // TODO
    } else {
      // Even: mirroring
      // TODO
    }
  } else if (address < 0xe000) {
    if (address.bit(0)) {
      // Odd: IRQ reload
      // TODO
    } else {
      // Even: IRQ latch
      // TODO
    }
  } else {
    // Addresses between $0xe000-$ffff
    if (address.bit(0)) {
      // Odd: IRQ enable
      // TODO
    } else {
      // Even: IRQ disable
      // TODO
    }
  }

  // TODO temporary assert
  assert(0);
}

// TODO: can throw exception on out-of-bounds memory access due to .at()
uint8 Mapper4::ReadCHR(uint16 address) {
  (void)address; // TODO remove me
  // TODO temporary assert
  assert(0);
  return 0;
}

void Mapper4::WriteCHR(uint16 address, uint8 value) {
  (void)address;
  (void)value; // TODO remove me
  // TODO: temporary assert
  assert(0);
}

} // namespace nesturbia
