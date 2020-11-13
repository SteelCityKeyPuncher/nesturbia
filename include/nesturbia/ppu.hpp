#ifndef NESTURBIA_PPU_HPP_INCLUDED
#define NESTURBIA_PPU_HPP_INCLUDED

#include <functional>

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

  struct ppuctrl_t {
    uint8_t nametable : 2;
    uint8 addressIncrement;
    uint16 spriteTableAddr;
    uint16 backgroundTableAddr;
    uint8 spriteHeight;
    // TODO handle bit 6
    bool generateNmiAtVBlank;

    auto &operator=(uint8 value) {
      nametable = value.bit(0) | (value.bit(1) << 1);
      addressIncrement = value.bit(2) ? 32U : 1U;
      spriteTableAddr = value.bit(3) ? 0x1000U : 0x0000U;
      backgroundTableAddr = value.bit(4) ? 0x1000U : 0x0000U;
      spriteHeight = value.bit(5) ? 16U : 8U;
      generateNmiAtVBlank = value.bit(7);

      return *this;
    }
  };

  struct ppumask_t {
    bool greyscale;
    bool showBackgroundInLeftmost8Px;
    bool showSpritesInLeftmost8Px;
    bool showBackground;
    bool showSprites;
    bool emphasizeRed;
    bool emphasizeGreen;
    bool emphasizeBlue;

    auto &operator=(uint8 value) {
      greyscale = value.bit(0);
      showBackgroundInLeftmost8Px = value.bit(1);
      showSpritesInLeftmost8Px = value.bit(2);
      showSprites = value.bit(3);
      showBackground = value.bit(4);
      emphasizeRed = value.bit(5);
      emphasizeGreen = value.bit(6);
      emphasizeBlue = value.bit(7);

      return *this;
    }
  };

  using chr_read_callback_t = std::function<uint8(uint16)>;
  using chr_write_callback_t = std::function<void(uint16, uint8)>;
  using set_pixel_callback_t = std::function<void(uint8, uint8, uint32_t)>;

  // Data
  // Registers
  ppuctrl_t ctrl;
  ppumask_t mask;
  uint8 status;
  uint8 oamaddr;

  uint16 scanline;
  uint16 dot;
  bool isOddFrame;

  bool writeLatch;
  uint8 latchedValue;

  // TODO initialize?
  addr_t vramAddr;
  addr_t vramAddrTemp;

  // Nametable memory
  std::array<uint8, 0x800> vram;

  // OAM memory
  std::array<uint8, 0x100> oam;

  chr_read_callback_t chrReadCallback;
  chr_write_callback_t chrWriteCallback;
  set_pixel_callback_t setPixelCallback;

  // Public functions
  Ppu(chr_read_callback_t chrReadCallback, chr_write_callback_t chrWriteCallback,
      set_pixel_callback_t setPixelCallback);

  bool Tick();
  uint8 ReadRegister(uint16 address);
  void WriteRegister(uint16 address, uint8 value);

  // Private functions
  uint8 read(uint16 address);
  void write(uint16 address, uint8 value);
};

} // namespace nesturbia

#endif // NESTURBIA_PPU_HPP_INCLUDED
