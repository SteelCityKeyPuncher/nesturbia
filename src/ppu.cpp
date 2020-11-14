#include <cassert>
#include <utility>

#include "nesturbia/ppu.hpp"

namespace nesturbia {

namespace {

constexpr std::array<uint32_t, 64> kRgbTable = {
    0x7c7c7c, 0x0000fc, 0x0000bc, 0x4428bc, 0x940084, 0xa80020, 0xa81000, 0x881400,
    0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
    0xbcbcbc, 0x0078f8, 0x0058f8, 0x6844fc, 0xd800cc, 0xe40058, 0xf83800, 0xe45c10,
    0xac7c00, 0x00b800, 0x00a800, 0x00a844, 0x008888, 0x000000, 0x000000, 0x000000,
    0xf8f8f8, 0x3cbcfc, 0x6888fc, 0x9878f8, 0xf878f8, 0xf85898, 0xf87858, 0xfca044,
    0xf8b800, 0xb8f818, 0x58d854, 0x58f898, 0x00e8d8, 0x787878, 0x000000, 0x000000,
    0xfcfcfc, 0xa4e4fc, 0xb8b8f8, 0xd8b8f8, 0xf8b8f8, 0xf8a4c0, 0xf0d0b0, 0xfce0a8,
    0xf8d878, 0xd8f878, 0xb8f8b8, 0xb8f8d8, 0x00fcfc, 0xf8d8f8, 0x000000, 0x000000};

uint16 nametableMap(Mapper::mirror_t mirrorType, uint16 address);

} // namespace

Ppu::Ppu(Cartridge &cartridge, nmi_callback_t nmiCallback)
    : cartridge(cartridge), nmiCallback(std::move(nmiCallback)) {
  Power();
}

void Ppu::Power() {
  ctrl = 0;
  mask = 0;

  status.latchedData = 0;
  status.spriteOverflow = true;
  status.sprite0Hit = false;
  status.vblankStarted = true;

  oamaddr = 0;

  scanline = 0;
  dot = 0;
  isOddFrame = false;

  addressWriteLatch = false;
  latchedValue = 0;
  readBuffer = 0;

  vramAddr.value = 0;
  vramAddrLatch.value = 0;

  fineX = 0;
}

bool Ppu::Tick() {
  // The return value from this function, which is used to determine when the frame is 'complete'
  // and ready to render
  bool updateFrame = false;

  if (scanline < 240 || scanline == 261) {
    // Line 0-239 (visible) / 261 (pre-rendering)
    // TODO clear secondary OAM

    if (scanline == 261 && dot == 1) {
      // Clear PPUSTATUS flags when (scanline == 261 / dot == 1)
      status.spriteOverflow = false;
      status.sprite0Hit = false;
      status.vblankStarted = false;
    }

    if (scanline < 240 && dot >= 2 && dot <= 257) {
      const uint8 x = dot - 2;
      uint8 paletteIndex = 0;

      if (mask.showBackground || mask.showSprites) {
        if (mask.showBackground) {
          // PPUMASK has a bit that prevents rendering the background in the first 8 pixels
          // If that bit isn't set, then don't render the background in those pixels
          if (mask.showBackgroundInLeftmost8Px || x >= 8) {
            paletteIndex |= renderData.bgShiftH.bit(15 - fineX) << 1;
            paletteIndex |= renderData.bgShiftL.bit(15 - fineX);

            if (paletteIndex) {
              paletteIndex |= renderData.atShiftH.bit(7 - fineX) << 3;
              paletteIndex |= renderData.atShiftL.bit(7 - fineX) << 2;
            }
          }
        }

        if (mask.showSprites) {
          // PPUMASK has a bit that prevents rendering sprites in the first 8 pixels
          // If that bit isn't set, then don't render sprites in those pixels
          if (mask.showSpritesInLeftmost8Px || x >= 8) {
          }
        }
      }

      // TODO could make a common function since this logic exists in PPUDATA code
      // TODO is this necessary based on code above?
      paletteIndex &= 0x1f;

      // $3f10/$3f14/$3f18/$3f1C are mirrors of $3f00/$3f04/$3f08/$3f0c
      if ((paletteIndex & 0x13) == 0x10) {
        paletteIndex &= ~0x10;
      }

      const auto rgb = kRgbTable[paletteRam[paletteIndex]];
      uint8 *pixel = &pixels[(scanline * 256 + x) * 3];

      pixel[0] = (rgb >> 16) & 0xff;
      pixel[1] = (rgb >> 8) & 0xff;
      pixel[2] = (rgb >> 0) & 0xff;
    }

    if ((dot >= 2 && dot <= 255) || (dot >= 322 && dot <= 337)) {
      renderData.bgShiftL <<= 1;
      renderData.bgShiftH <<= 1;
      renderData.atShiftL = (renderData.atShiftL << 1) | renderData.atLatchL;
      renderData.atShiftH = (renderData.atShiftH << 1) | renderData.atLatchH;

      switch (dot % 8) {
      case 1:
        renderData.address = 0x2000 | (vramAddr.value & 0xfff);
        renderData.bgShiftL = (renderData.bgShiftL & 0xff00) | renderData.bgL;
        renderData.bgShiftH = (renderData.bgShiftH & 0xff00) | renderData.bgH;

        renderData.atLatchL = renderData.attributeByte.bit(0);
        renderData.atLatchH = renderData.attributeByte.bit(1);
        break;

      case 2:
        renderData.nametableByte = read(renderData.address);
        break;

      case 3:
        renderData.address = 0x23c0;
        renderData.address |= vramAddr.fields.nametable << 10;
        renderData.address |= (vramAddr.fields.coarseY >> 2) << 3;
        renderData.address |= (vramAddr.fields.coarseX >> 2);
        break;

      case 4:
        renderData.attributeByte = read(renderData.address);
        renderData.attributeByte >>= vramAddr.fields.coarseY & 0x2 ? 4 : 0;
        renderData.attributeByte >>= vramAddr.fields.coarseX & 0x2 ? 2 : 0;
        break;

      case 5:
        renderData.address =
            ctrl.backgroundTableAddr + (renderData.nametableByte << 4) + vramAddr.fields.fineY;
        break;

      case 6:
        renderData.bgL = read(renderData.address);
        break;

      case 7:
        renderData.address += 8;
        break;

      case 0:
        renderData.bgH = read(renderData.address);

        if (mask.showBackground || mask.showSprites) {
          if (vramAddr.fields.coarseX == 31) {
            vramAddr.value ^= 0x41f;
          } else {
            vramAddr.fields.coarseX++;
          }
        }
        break;
      }
    } else if (dot == 256) {
      renderData.bgH = read(renderData.address);

      if (mask.showBackground || mask.showSprites) {
        if (++vramAddr.fields.fineY == 0x0) {
          if (vramAddr.fields.coarseY == 0x1f) {
            vramAddr.fields.coarseY = 0;
          } else if (vramAddr.fields.coarseY == 0x1d) {
            vramAddr.fields.coarseY = 0;
            vramAddr.fields.nametable ^= 0x2;
          } else {
            ++vramAddr.fields.coarseY;
          }
        }
      }
    } else if (dot == 257) {
      renderData.atLatchL = renderData.attributeByte.bit(0);
      renderData.atLatchH = renderData.attributeByte.bit(1);

      if (mask.showBackground || mask.showSprites) {
        vramAddr.fields.coarseX = vramAddrLatch.fields.coarseX;

        vramAddr.fields.nametable &= 0x2;
        vramAddr.fields.nametable |= vramAddrLatch.fields.nametable & 0x1;
      }
    } else if (dot >= 280 && dot <= 304) {
      if (scanline == 261 && (mask.showBackground || mask.showSprites)) {
        vramAddr.fields.coarseY = vramAddrLatch.fields.coarseY;

        vramAddr.fields.nametable &= 0x1;
        vramAddr.fields.nametable |= vramAddrLatch.fields.nametable & 0x2;

        vramAddr.fields.fineY = vramAddrLatch.fields.fineY;
      }
    } else if (dot == 1 || dot == 321 || dot == 339) {
      renderData.address = 0x2000 | (vramAddr.value & 0xfff);
    } else if (dot == 338 || dot == 340) {
      renderData.nametableByte = read(renderData.address);
    }
  } else if (scanline == 240 && dot == 0) {
    // Line 240: The frame is considered ready for the frontend on dot 0
    updateFrame = true;
  } else if (scanline == 241 && dot == 1) {
    // Line 241: Set NMI on dot 1
    // Set VBLANK (bit 7) in PPUSTATUS ($2002)
    status.vblankStarted = true;

    // If PPUCTRL has the bit set to trigger an NMI on the CPU, do so now
    if (ctrl.generateNmiAtVBlank && nmiCallback) {
      nmiCallback();
    }
  }

  // Increment the counters for the current dot + scanline
  if (++dot == 341) {
    dot = 0;

    if (++scanline == 262) {
      // Odd frames skip the first dot on the first scanline if currently rendering
      if ((mask.showBackground || mask.showSprites) && isOddFrame) {
        ++dot;
      }

      scanline = 0;
      isOddFrame = !isOddFrame;
    }
  }

  return updateFrame;
}

uint8 Ppu::ReadRegister(uint16 address) {
  assert(address >= 0x2000 && address < 0x4000);

  switch (address & 0x7) {
  case 2:
    // The bottom 5 bits are the same as the latched value
    status.latchedData = latchedValue & 0x1f;
    latchedValue = status;

    // Reading this register clears the address write latch
    addressWriteLatch = false;
    break;

  case 4:
    latchedValue = oam[oamaddr];
    break;

  case 7:
    // PPUDATA ($2007)
    // Palette memory ($3f00-$3eff) is internal to the PPU so it can be read in one cycle
    // All other memory read from the PPU must be buffered, taking two cycles to read
    if (vramAddr.address < 0x3f00) {
      // The returned value is from the internal buffer
      latchedValue = readBuffer;

      // Store the actual value in the buffer for the next read
      readBuffer = read(vramAddr.address);
    } else {
      // The palette memory can be returned immediately without being buffered
      latchedValue = read(vramAddr.address);
    }

    vramAddr.address += ctrl.addressIncrement;
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
    vramAddrLatch.fields.nametable = ctrl.nametable;
    break;

  case 1:
    mask = value;
    break;

  case 2:
    // PPUSTATUS is read-only
    // Nothing to do here
    break;

  case 3:
    oamaddr = value;
    break;

  case 4:
    oam[oamaddr++] = value;
    break;

  case 5:
    // PPUSCROLL ($2005)
    if (!addressWriteLatch) {
      // First write
      // Coarse X is upper 5 bits
      vramAddrLatch.fields.coarseX = value >> 3;

      // Fine X is lower 3 bits
      fineX = value & 0x7;
    } else {
      // Second write
      // Coarse Y is upper 5 bits
      vramAddrLatch.fields.coarseY = value >> 3;

      // Fine Y is lower 3 bits
      vramAddrLatch.fields.fineY = value & 0x7;
    }

    // Each write to this register (and PPUADDR/$2006) causes this flip-flop to toggle
    addressWriteLatch = !addressWriteLatch;
    break;

  case 6:
    // PPUADDR ($2006)
    if (!addressWriteLatch) {
      // First write
      vramAddrLatch.value &= 0xff;
      vramAddrLatch.value |= ((value & 0x3f) << 8);
    } else {
      // Second write
      vramAddrLatch.value &= 0xff00;
      vramAddrLatch.value |= value;
      vramAddr = vramAddrLatch;
    }

    // Each write to this register (and PPUSCROLL/$2005) causes this flip-flop to toggle
    addressWriteLatch = !addressWriteLatch;
    break;

  case 7:
  default:
    // PPUDATA ($2007)
    if (vramAddr.address < 0x2000) {
      // Write mapper CHR-ROM/RAM
      cartridge.WriteChr(vramAddr.address, value);
    } else if (vramAddr.address < 0x3f00) {
      // Nametable RAM
      vram[nametableMap(cartridge.GetMirrorType(), vramAddr.address)] = value;
    } else {
      // Palette memory ($3f00-$3eff)
      // $3f00-$3f1f are mirrored up to $3fff
      auto paletteAddr = vramAddr.address & 0x1f;

      // $3f10/$3f14/$3f18/$3f1C are mirrors of $3f00/$3f04/$3f08/$3f0c
      if ((paletteAddr & 0x13) == 0x10) {
        paletteAddr &= ~0x10;
      }

      // Only 6-bit values should be here since reading the top 2 bits should return 0
      paletteRam[paletteAddr] = value & 0x3f;
    }

    vramAddr.address += ctrl.addressIncrement;
    break;
  }
}

// TODO put in anonymous namespace?
uint8 Ppu::read(uint16 address) {
  assert(address < 0x4000);

  if (address < 0x2000) {
    // Read mapper CHR-ROM/RAM
    return cartridge.ReadChr(address);
  }

  if (address < 0x3f00) {
    // Nametable RAM
    return vram[nametableMap(cartridge.GetMirrorType(), address)];
  }

  // Palette memory ($3f00-$3eff)
  // $3f00-$3f1f are mirrored up to $3fff
  auto paletteAddr = address & 0x1f;

  // $3f10/$3f14/$3f18/$3f1C are mirrors of $3f00/$3f04/$3f08/$3f0c
  if ((paletteAddr & 0x13) == 0x10) {
    paletteAddr &= ~0x10;
  }

  return paletteRam[paletteAddr];
}

namespace {

uint16 nametableMap(Mapper::mirror_t mirrorType, uint16 address) {
  switch (mirrorType) {
  case Mapper::mirror_t::horizontal:
    // Horizontal mapping ties bit A11 to A10
    return ((address >> 1) & 0x400) | (address & 0x3ff);

  case Mapper::mirror_t::vertical:
  default:
    return address & 0x7ff;
  }
}

} // namespace

} // namespace nesturbia
