#ifndef NESTURBIA_PPU_HPP_INCLUDED
#define NESTURBIA_PPU_HPP_INCLUDED

#include <array>
#include <functional>

#include "nesturbia/cartridge.hpp"
#include "nesturbia/types.hpp"

namespace nesturbia {

struct Ppu {
  // Constants
  static inline constexpr auto kScreenWidth = 256U;
  static inline constexpr auto kScreenHeight = 240U;

  // Types
  union addr_t {
    struct {
      unsigned coarseX : 5;
      unsigned coarseY : 5;
      unsigned nametable : 2;
      unsigned fineY : 3;
    } fields;

    unsigned value : 15;
    unsigned address : 14;
  };

  struct ppuctrl_t {
    uint8_t nametable : 2;
    uint8 addressIncrement;
    uint16 spriteTableAddr;
    uint16 backgroundTableAddr;
    uint8 spriteHeight;
    // TODO handle bit 6?
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
    bool grayscale;
    bool showBackgroundInLeftmost8Px;
    bool showSpritesInLeftmost8Px;
    bool showBackground;
    bool showSprites;
    bool emphasizeRed;
    bool emphasizeGreen;
    bool emphasizeBlue;

    auto &operator=(uint8 value) {
      grayscale = value.bit(0);
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

  struct ppustatus_t {
    uint8_t latchedData : 5;
    bool spriteOverflow;
    bool sprite0Hit;
    bool vblankStarted;

    operator unsigned() const {
      return latchedData | spriteOverflow << 5 | sprite0Hit << 6 | vblankStarted << 7;
    }
  };

  // TODO temporary
  struct render_data_t {
    uint16 address;
    uint8 nametableByte;
    uint8 attributeByte;
    uint8 bgL;
    uint8 bgH;
    uint16 bgShiftL;
    uint16 bgShiftH;
    uint8 atShiftL;
    uint8 atShiftH;
    bool atLatchL;
    bool atLatchH;
  };

  struct oam_entry_t {
    uint8 id;
    uint8 x;
    uint8 y;
    uint8 tile;
    uint8 attributes;
    uint8 dataL;
    uint8 dataH;
  };

  using nmi_callback_t = std::function<void(void)>;

  // Data
  // Cartridge reference
  Cartridge &cartridge;

  // Registers
  ppuctrl_t ctrl;
  ppumask_t mask;
  ppustatus_t status;
  uint8 oamaddr;

  uint16 scanline;
  uint16 dot;
  bool isOddFrame;

  bool addressWriteLatch;

  // TODO rename
  uint8 latchedValue;
  uint8 readBuffer;

  // The actual VRAM address (V)
  addr_t vramAddr;

  // The VRAM address latch (T)
  addr_t vramAddrLatch;

  uint8 fineX;

  // Nametable memory
  std::array<uint8, 0x800> vram;

  // OAM memory
  std::array<uint8, 0x100> oam;
  std::array<oam_entry_t, 8> oamPrimary;
  std::array<oam_entry_t, 8> oamSecondary;

  // Palette memory
  std::array<uint8, 0x20> paletteRam;

  // Pixel memory
  std::array<uint8, kScreenWidth * kScreenHeight * 3> pixels;

  // Function that's called when an NMI is encountered
  // This is called when VBLANK occurs, and the appropriate bit (7) is set in PPUCTRL
  nmi_callback_t nmiCallback;

  // TODO temporary
  render_data_t renderData;

  // Public functions
  Ppu(Cartridge &cartridge, nmi_callback_t nmiCallback);

  void Power();
  bool Tick();
  uint8 ReadRegister(uint16 address);
  void WriteRegister(uint16 address, uint8 value);

  // Private functions
  uint8 read(uint16 address);
};

} // namespace nesturbia

#endif // NESTURBIA_PPU_HPP_INCLUDED
