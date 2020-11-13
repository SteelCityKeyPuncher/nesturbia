#include <cassert>
#include <utility>

#include "nesturbia/ppu.hpp"

namespace nesturbia {

Ppu::Ppu(Cartridge &cartridge) : cartridge(cartridge) {}

bool Ppu::Tick() {
  // TODO
  if (scanline == 240 && dot == 0) {
    static uint32_t z = 0;
    for (uint16_t xx = 0; xx < 256; xx++) {
      for (uint16_t yy = 0; yy < 240; yy++) {
        // TODO bounds check
        uint8 *pixel = &pixels[(yy * 256 + xx) * 3];

        *(pixel++) = 0;
        *(pixel++) = 0;
        *pixel++ = z;
        z += rand();
      }
    }

    // Return true so that the PPU pixels are written to the screen
    return true;
  }

  if (++dot == 341) {
    dot = 0;
    if (++scanline == 262) {
      scanline = 0;
      isOddFrame = !isOddFrame;
    }
  }

  // Not yet ready to update the screen
  return false;
}

uint8 Ppu::ReadRegister(uint16 address) {
  assert(address >= 0x2000 && address < 0x4000);

  switch (address & 0x7) {
  case 2:
    // The bottom 5 bits are the same as the latched value
    status.latchedData = latchedValue & 0x1f;
    latchedValue = status;

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

void Ppu::WriteRegister(uint16 address, uint8 value) {
  assert(address >= 0x2000 && address < 0x4000);

  // Latch the last written value
  // If a write-only register is read, this value is read instead
  latchedValue = value;

  // Mirror 0x2000-0x2007 in the range 0x2008-0x3fff
  switch (address & 0x7) {
  case 0:
    ctrl = value;
    vramAddrTemp.fields.nametable = ctrl.nametable;
    break;

  case 1:
    mask = value;
    break;

  case 2:
    // PPUSTATUS is read-only (nothing to do here)
    break;

  case 3:
    oamaddr = value;
    break;

  case 4:
    oam[oamaddr++] = value;
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

uint8 Ppu::read(uint16 address) {
  if (address < 0x2000) {
    // Read mapper CHR-ROM/RAM
    return cartridge.ReadChr(address);
  }

  if (address < 0x3f00) {
    return 0;
  }

  if (address < 0x4000) {
    return 0;
  }

  return 0;
}

void Ppu::write(uint16 address, uint8 value) {
  if (address < 0x2000) {
    // Write mapper CHR-ROM/RAM
    return cartridge.WriteChr(address, value);
  }

  if (address < 0x3f00) {
    return;
  }

  if (address < 0x4000) {
    return;
  }
}

} // namespace nesturbia
