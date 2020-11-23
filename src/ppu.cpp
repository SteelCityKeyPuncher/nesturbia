#include <cassert>
#include <utility>

#include "nesturbia/ppu.hpp"

namespace nesturbia {

namespace {

// TODO: see if this is 'correct' and allow other palettes to be used
constexpr std::array<uint32_t, 64> kRgbTable = {
    0x545454, 0x001e74, 0x081090, 0x300088, 0x440064, 0x5c0030, 0x540400, 0x3c1800,
    0x202a00, 0x083a00, 0x004000, 0x003c00, 0x00323c, 0x000000, 0x000000, 0x000000,
    0x989698, 0x084cc4, 0x3032ec, 0x5c1ee4, 0x8814b0, 0xa01464, 0x982220, 0x783c00,
    0x545a00, 0x287200, 0x087c00, 0x007628, 0x006678, 0x000000, 0x000000, 0x000000,
    0xeceeec, 0x4c9aec, 0x787cec, 0xb062ec, 0xe454ec, 0xec58b4, 0xec6a64, 0xd48820,
    0xa0aa00, 0x74c400, 0x4cd020, 0x38cc6c, 0x38b4cc, 0x3c3c3c, 0x000000, 0x000000,
    0xeceeec, 0xa8ccec, 0xbcbcec, 0xd4b2ec, 0xecaeec, 0xecaed4, 0xecb4b0, 0xe4c490,
    0xccd278, 0xb4de78, 0xa8e290, 0x98e2b4, 0xa0d6e4, 0xa0a2a0, 0x000000, 0x000000};

uint16 nametableMap(Mapper::mirror_t mirrorType, uint16 address);

} // namespace

Ppu::Ppu(Cartridge &cartridge, nmi_callback_t nmiCallback)
    : cartridge(cartridge), nmiCallback(std::move(nmiCallback)) {}

void Ppu::Power() {
  ctrl = 0;
  mask = 0;

  status.latchedData = 0;
  status.spriteOverflow = true;
  status.sprite0Hit = false;

  // TODO: this is 'often set' on boot
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
    if (scanline == 261 && dot == 1) {
      // Clear PPUSTATUS flags when (scanline == 261 / dot == 1)
      status.spriteOverflow = false;
      status.sprite0Hit = false;
      status.vblankStarted = false;
    }

    if (scanline < 240 && dot >= 2 && dot <= 257) {
      const uint8 x = dot - 2;
      uint8 paletteIndex = 0;
      uint8 objPalette = 0;
      bool objPriority = false;

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
            for (int i = 7; i >= 0; i--) {
              if (oamPrimary[i].id == 64) {
                // Empty entry
                continue;
              }

              unsigned sprX = x - oamPrimary[i].x;
              if (sprX >= 8) {
                // Out of range
                continue;
              }

              if (oamPrimary[i].attributes.bit(6)) {
                // Horizontal flipping
                sprX ^= 7;
              }

              uint8 sprPalette = oamPrimary[i].dataH.bit(7 - sprX) << 1;
              sprPalette |= oamPrimary[i].dataL.bit(7 - sprX);
              if (sprPalette == 0) {
                // Transparent pixel
                continue;
              }

              if (oamPrimary[i].id == 0 && paletteIndex && x != 255) {
                status.sprite0Hit = true;
              }

              sprPalette |= (oamPrimary[i].attributes & 3) << 2;
              objPalette = sprPalette | 0x10;
              objPriority = oamPrimary[i].attributes.bit(5);
            }
          }
        }
      }

      if (objPalette && (paletteIndex == 0 || !objPriority)) {
        paletteIndex = objPalette;
      }

      const auto rgb = kRgbTable[read(0x3f00 | paletteIndex)];
      uint8 *pixel = &pixels[(scanline * 256 + x) * 3];

      pixel[0] = (rgb >> 16) & 0xff;
      pixel[1] = (rgb >> 8) & 0xff;
      pixel[2] = (rgb >> 0) & 0xff;
    }

    if (dot == 1) {
      for (auto &entry : oamSecondary) {
        entry.id = 64;
        entry.y = 0xFF;
        entry.tile = 0xFF;
        entry.attributes = 0xFF;
        entry.x = 0xFF;
        entry.dataL = 0;
        entry.dataH = 0;
      }

      renderData.address = 0x2000 | (vramAddr.value & 0xfff);
    } else if ((dot >= 2 && dot <= 255) || (dot >= 322 && dot <= 337)) {
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
      if (scanline != 261) {
        // Evaluate sprites
        uint8 spriteCount = 0;
        for (uint8 i = 0; i < 64; i++) {
          auto y = oam[i * 4 + 0];
          int row = (int)scanline - y;
          if (row < 0 || row >= ctrl.spriteHeight) {
            continue;
          }

          oamSecondary[spriteCount].id = i;
          oamSecondary[spriteCount].y = oam[i * 4 + 0];
          oamSecondary[spriteCount].tile = oam[i * 4 + 1];
          oamSecondary[spriteCount].attributes = oam[i * 4 + 2];
          oamSecondary[spriteCount].x = oam[i * 4 + 3];

          if (++spriteCount == 8) {
            status.spriteOverflow = true;
            break;
          }
        }
      }

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
    } else if (dot == 321) {
      // Load sprites
      for (uint8 i = 0; i < 8; i++) {
        oamPrimary[i] = oamSecondary[i];

        uint16 address;
        if (ctrl.spriteHeight == 16) {
          address = ((oamPrimary[i].tile & 1) * 0x1000) + ((oamPrimary[i].tile & ~1) * 16);
        } else {
          address = ctrl.spriteTableAddr + (oamPrimary[i].tile * 16);
        }

        auto spriteY = (scanline - oamPrimary[i].y) & (ctrl.spriteHeight - 1);
        if (oamPrimary[i].attributes.bit(7)) {
          // Vertical flipping
          spriteY ^= (ctrl.spriteHeight - 1);
        }

        address += spriteY + (spriteY & 8);

        oamPrimary[i].dataL = read(address);
        oamPrimary[i].dataH = read(address + 8);
      }

      renderData.address = 0x2000 | (vramAddr.value & 0xfff);
    } else if (dot == 338 || dot == 340) {
      renderData.nametableByte = read(renderData.address);
    } else if (dot == 339) {
      renderData.address = 0x2000 | (vramAddr.value & 0xfff);
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

  // $2008-$3fff are mirrors of $2000-$2007
  switch (0x2000 | (address & 0x7)) {
  case 0x2002:
    // PPUSTATUS
    // The bottom 5 bits are pulled from the latched read/write value
    status.latchedData = latchedValue & 0x1f;
    latchedValue = status;

    // Reading this register clears the address write latch
    addressWriteLatch = false;
    break;

  case 0x2004:
    // OAMADDR
    latchedValue = oam[oamaddr];
    break;

  case 0x2007:
    // PPUDATA
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
  }

  return latchedValue;
}

void Ppu::WriteRegister(uint16 address, uint8 value) {
  assert(address >= 0x2000 && address < 0x4000);

  // Latch the last written value
  // If a write-only register is read, this value is read instead
  latchedValue = value;

  // $2008-$3fff are mirrors of $2000-$2007
  switch (0x2000 | (address & 0x7)) {
  case 0x2000:
    // PPUCTRL
    ctrl = value;
    vramAddrLatch.fields.nametable = ctrl.nametable;
    break;

  case 0x2001:
    // PPUMASK
    mask = value;
    break;

  case 0x2002:
    // PPUSTATUS (read-only)
    break;

  case 0x2003:
    // OAMADDR
    oamaddr = value;
    break;

  case 0x2004:
    // OAMDATA
    oam[oamaddr++] = value;
    break;

  case 0x2005:
    // PPUSCROLL
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

  case 0x2006:
    // PPUADDR
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

  case 0x2007:
    // PPUDATA
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

      // $3f10/$3f14/$3f18/$3f1c are mirrors of $3f00/$3f04/$3f08/$3f0c
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

  // $3f10/$3f14/$3f18/$3f1c are mirrors of $3f00/$3f04/$3f08/$3f0c
  if ((paletteAddr & 0x13) == 0x10) {
    paletteAddr &= ~0x10;
  }

  return paletteRam[paletteAddr];
}

namespace {

uint16 nametableMap(Mapper::mirror_t mirrorType, uint16 address) {
  switch (mirrorType) {
  case Mapper::mirror_t::horizontal:
    // Horizontal mapping ties address bit A11 to A10 in the mapper
    return ((address >> 1) & 0x400) | (address & 0x3ff);

  case Mapper::mirror_t::vertical:
    return address & 0x7ff;

  case Mapper::mirror_t::oneScreenLower:
    return address & 0x3ff;

  case Mapper::mirror_t::oneScreenHigher:
    return 0x400 | (address & 0x3ff);

  default:
    // TODO: handle other types for MMC1 (and potentially others)
    return address & 0x7ff;
  }
}

} // namespace

} // namespace nesturbia
