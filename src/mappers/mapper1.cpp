#include <cassert>

#include "nesturbia/mappers/mapper1.hpp"

namespace nesturbia {

Mapper::ptr_t Mapper1::Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom) {
  // TODO: Validate PRG-ROM size?
  auto mapper = std::make_unique<Mapper1>();

  mapper->prgRom = prgRom;
  mapper->chrRom = chrRom;

  if (chrRom.empty()) {
    mapper->chrRam.resize(0x2000);
  }

  return mapper;
}

Mapper::mirror_t Mapper1::GetMirrorType() const {
  constexpr std::array<mirror_t, 4> kMirrorTypes = {mirror_t::oneScreenLower,
                                                    mirror_t::oneScreenHigher, mirror_t::vertical,
                                                    mirror_t::horizontal};

  return kMirrorTypes[controlRegister.mirrorType];
}

uint8 Mapper1::ReadPRG(uint16 address) {
  assert(address >= 0x8000);

  uint8 page16KLow;
  uint8 page16KHigh;

  switch (controlRegister.prgRomBankMode) {
  case 0x0:
  case 0x1:
    // 32 KB mode
    // Bit 0 is the 16 KB offset, so ignore it to get the 32 KB page on which it resides
    page16KLow = prgBankRegister.prgRomBank & 0xe;
    page16KHigh = page16KLow | 0x1;
    break;

  case 0x2:
    // Fix the first bank at $8000
    page16KLow = 0;

    // Switch 16 KB bank at $c000
    page16KHigh = prgBankRegister.prgRomBank;
    break;

  case 0x3:
    // Switch 16 KB bank at $8000
    page16KLow = prgBankRegister.prgRomBank;

    // Fix the last bank at $c000
    page16KHigh = (prgRom.size() >> 14) - 1;
    break;
  }

  // TODO: using .at() for now to detect out of bounds memory access
  if (address < 0xc000) {
    // "Low" address
    return prgRom.at((page16KLow << 14) | (address & 0x3fff));
  }

  // "High" address
  return prgRom.at((page16KHigh << 14) | (address & 0x3fff));
}

void Mapper1::WritePRG(uint16 address, uint8 value) {
  assert(address >= 0x8000);

  // TODO: writes to the serial port on consecutive cycles are ignored
  // See https://wiki.nesdev.com/w/index.php/MMC1 for more information
  if (value.bit(7)) {
    // Reset the shift register
    // It takes 5 writes before the shift register value is 'committed'
    // Instead of having a write counter, use the fact that the 0th bit will have shifted right by 5
    shiftRegister = 0x10;

    // TODO other stuff here?
    return;
  }

  // Writing a value to the shift register
  // Check to see if this is the 5th write to the shift register since it's been reset
  // On the 5th write, the shift register contains the final value
  const auto is5thWrite = shiftRegister.bit(0);

  // Perform the shift
  shiftRegister = (shiftRegister >> 1) | (value.bit(0) << 4);

  if (is5thWrite) {
    // Bits 14 and 13 are used to determine which register to write
    switch ((address >> 13) & 0x3) {
    case 0b00:
      // $8000-$9fff: control
      // CHR-ROM bank mode (bit 4)
      controlRegister.chrRomBankMode = shiftRegister.bit(4);

      // PRG-ROM bank mode (bits 3 and 2)
      controlRegister.prgRomBankMode = (shiftRegister >> 2) & 0x3;

      // Mirror type (bits 1 and 0)
      controlRegister.mirrorType = shiftRegister & 0x3;
      break;

    case 0b01:
      // $a000-$bfff: CHR bank 0
      chrBankRegisters[0] = shiftRegister;
      break;

    case 0b10:
      // $c000-dfff: CHR bank1
      chrBankRegisters[1] = shiftRegister;
      break;

    case 0b11:
      // $e000-$ffff: PRG bank
      prgBankRegister.prgRamChipEnable = shiftRegister.bit(4);

      // TODO: handle PRG-RAM
      prgBankRegister.prgRomBank = shiftRegister & 0xf;
      break;
    }

    // Clear the shift register to its reset state after the 5th write
    shiftRegister = 0x10;
  }
}

uint8 Mapper1::ReadCHR(uint16 address) {
  // TODO: make more efficient
  if (!chrRam.empty()) {
    // TODO: using .at() to detect out of bounds memory accesses
    return chrRam.at(address);
  }

  if (address < chrRom.size()) {
    return chrRom[address];
  }

  // TODO: testing
  assert(0);
  return 0;
}

void Mapper1::WriteCHR(uint16 address, uint8 value) {
  // TODO: make more efficient
  if (!chrRam.empty()) {
    // TODO: using .at() to detect out of bounds memory accesses
    chrRam.at(address) = value;
    return;
  }

  if (address < chrRom.size()) {
    chrRom[address] = value;
    return;
  }

  // TODO: testing
  assert(0);
}

} // namespace nesturbia
