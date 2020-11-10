#ifndef NESTURBIA_PPU_HPP_INCLUDED
#define NESTURBIA_PPU_HPP_INCLUDED

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Ppu {
  // Types
  union addr_t {
    struct {
      unsigned coarseX : 5;
      unsigned coarseY : 5;
      unsigned nametable : 2;
      unsigned fineY : 3;
    } fields;

    unsigned val : 15;
    unsigned addr : 14;
  };

  // Data
  // Registers
  uint8 ctrl = 0;
  uint8 mask = 0;
  uint8 status = 0;
  uint8 oamaddr = 0;
  uint8 oamdata = 0;

  uint16_t x = 0;
  uint16_t y = 0;
  bool isOddFrame = false;

  bool writeLatch = false;
  uint8 latchedValue = 0;

  // TODO initialize?
  addr_t vramAddr;
  addr_t vramAddrTemp;

  // Public functions
  void Tick();
  uint8 Read(uint16 address);
  void Write(uint16 address, uint8 value);

  // Private functions
};

} // namespace nesturbia

#endif // NESTURBIA_PPU_HPP_INCLUDED
