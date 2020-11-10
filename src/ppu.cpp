#include <cassert>

#include "nesturbia/ppu.hpp"

namespace nesturbia {

void Ppu::Tick() {
  // TODO

  if (++x == 341) {
    x = 0;
    if (++y == 262) {
      y = 0;
      isOddFrame = !isOddFrame;
    }
  }
}

uint8 Ppu::Read(uint16 address) {
  assert(address >= 0x2000 && address < 0x4000);

  switch (address & 0x7) {
  case 2:
    // Bottom 5 bits are not updated for this register
    latchedValue &= 0x1f;
    latchedValue |= (status & 0xe0);

    // Reading this register clears the write latch
    writeLatch = false;
    break;

  case 4:
    // TODO read OAM
    break;

  case 7:
    // TODO PPUDATA
    break;

  default:
    break;
  }

  return latchedValue;
}

void Ppu::Write(uint16 address, uint8 value) {
  assert(address >= 0x2000 && address < 0x4000);

  // Latch the last written value
  // If a write-only register is read, this value is read instead
  latchedValue = value;

  // Mirror 0x2000-0x2007 in the range 0x2008-0x3fff
  switch (address & 0x7) {
  case 0:
    ctrl = value;
    vramAddrTemp.fields.nametable = ctrl & 0x3;
    break;

  case 1:
    mask = value;
    break;

  case 2:
    // PPUSTATUS is read-only
    break;

  case 3:
    oamaddr = value;
    break;

  case 4:
    oamdata = value;
    break;

  case 5:
    if (!writeLatch) {
      // First write
      vramAddrTemp.fields.coarseX = value >> 3;
      // TODO fine X
    } else {
      vramAddrTemp.fields.coarseY = value >> 3;
      vramAddrTemp.fields.fineY = value & 0x7;
    }

    writeLatch = !writeLatch;
    break;

  case 6:
    if (!writeLatch) {
      // First write
      vramAddrTemp.val &= 0xff;
      vramAddrTemp.val |= ((value & 0x3f) << 8);
    } else {
      // Second write
      vramAddrTemp.val &= 0xff00;
      vramAddrTemp.val |= value;
      vramAddr = vramAddrTemp;
    }

    writeLatch = !writeLatch;
    break;

  case 7:
  default:
    // TODO PPUDATA
    break;
  }
}

} // namespace nesturbia
