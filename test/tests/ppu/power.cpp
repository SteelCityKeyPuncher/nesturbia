#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cartridge.hpp"
#include "nesturbia/ppu.hpp"
using namespace nesturbia;

TEST_CASE("Ppu_Power", "[ppu]") {
  // Test the power-up state of the PPU
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  //
  // PPUCTRL ($2000) == 0x00
  //

  // Bits 1-0 (nametable)
  CHECK(ppu.ctrl.nametable == 0x0);

  // Bit 2 (address increment): 0:1, 1:32
  CHECK(ppu.ctrl.addressIncrement == 1);

  // Bit 3 (sprite pattern table address): 0:0x0000, 1:0x1000
  CHECK(ppu.ctrl.spriteTableAddr == 0x0000);

  // Bit 4 (background pattern table address): 0:0x0000, 1:0x1000
  CHECK(ppu.ctrl.backgroundTableAddr == 0x0000);

  // Bit 5 (sprite height): 0:8 pixels, 1:16 pixels
  CHECK(ppu.ctrl.spriteHeight == 8);

  // Bit 6 (PPU master/slave select): ignored for now (TODO)

  // Bit 7 (generate NMI at start of VBLANK): 0:false, 1:true
  CHECK(ppu.ctrl.generateNmiAtVBlank == false);

  //
  // PPUMASK ($2001) == 0x00
  //

  // Bit 0 (grayscale) - bool
  CHECK(ppu.mask.grayscale == false);

  // Bit 1 (whether the background should be shown in the first 8 pixels on the left) - bool
  CHECK(ppu.mask.showBackgroundInLeftmost8Px == false);

  // Bit 2 (whether sprites should be shown in the first 8 pixels on the left) - bool
  CHECK(ppu.mask.showSpritesInLeftmost8Px == false);

  // Bit 3 (whether the background should be displayed) - bool
  CHECK(ppu.mask.showBackground == false);

  // Bit 4 (whether sprites should be displayed) - bool
  CHECK(ppu.mask.showSprites == false);

  // Bit 5 (emphasize red colors) - bool
  CHECK(ppu.mask.emphasizeRed == false);

  // Bit 6 (emphasize green colors) - bool
  CHECK(ppu.mask.emphasizeGreen == false);

  // Bit 7 (emphasize blue colors) - bool
  CHECK(ppu.mask.emphasizeBlue == false);

  //
  // PPUSTATUS ($2002) == 0x00
  //

  // Bit 7 of PPUSTATUS is "often set" after a power up
  // In this emulator, it is always set on a power up
  CHECK(ppu.status.vblankStarted == true);

  // Bit 6 of PPUSTATUS is zero after a power up
  CHECK(ppu.status.sprite0Hit == false);

  // Bit 5 of PPUSTATUS is "often set" after a power up
  // In this emulator, it is always set on a power up
  CHECK(ppu.status.spriteOverflow == true);
}
