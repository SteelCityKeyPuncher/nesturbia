#ifndef NESTURBIA_MAPPERS_MAPPER_1_HPP_INCLUDED
#define NESTURBIA_MAPPERS_MAPPER_1_HPP_INCLUDED

#include <vector>

#include "nesturbia/mapper.hpp"

namespace nesturbia {

// Mapper 0: aka SxROM (includes SAROM, SBROM, SHROM, etc.)
struct Mapper1 : public Mapper {
  // Data
  std::vector<uint8> prgRom;
  std::vector<uint8> chrRom;
  std::vector<uint8> chrRam;

  // Shift register that's modified when $8000-$ffff are written
  // If the value written has the top/7th bit set, the shift register is reset
  // Otherwise, the bottom/0th bit is shifted in
  // 5 writes after a reset of this register causes an internal register to be written
  // 5-bit value
  uint8 shiftRegister = 0x10;

  // Control register: internal register at $8000-$9fff
  struct {
    // 2-bit value
    uint8 mirrorType = 0;

    // TODO: this is 'usually' correct, although sometimes on hardware the bank mapping is different
    // 2-bit value
    uint8 prgRomBankMode = 3;

    // 1-bit value
    bool chrRomBankMode = false;
  } controlRegister;

  // CHR0/CHR1 bank registers:
  // * CHR0 bank: internal register at $a000-$bfff
  // * CHR1 bank: internal register at $c000-$dfff
  // 5-bit values
  std::array<uint8, 2> chrBankRegisters;

  // PRG bank register: internal register at $e000-$ffff
  struct {
    // PRG-RAM chip enable (bit 4)
    bool prgRamChipEnable = false;

    // PRG-ROM bank (bits 3 to 0)
    // 4-bit value
    uint8 prgRomBank = 0;
  } prgBankRegister;

  // Public functions
  static Mapper::ptr_t Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom);

  [[nodiscard]] mirror_t GetMirrorType() const override;

  uint8 ReadPRG(uint16 address) override;
  void WritePRG(uint16 address, uint8 value) override;

  uint8 ReadCHR(uint16 address) override;
  void WriteCHR(uint16 address, uint8 value) override;
};

} // namespace nesturbia

#endif // NESTURBIA_MAPPERS_MAPPER_1_HPP_INCLUDED
