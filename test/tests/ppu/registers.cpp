#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cartridge.hpp"
#include "nesturbia/ppu.hpp"
using namespace nesturbia;

TEST_CASE("Ppu_RegPpuctrl", "[ppu]") {
  // Test various PPUCTRL register settings
  nesturbia::Cartridge cartridge;
  nesturbia::Ppu ppu(cartridge, [](uint8, uint8, uint32_t) {});

  // Write directly to 0x2000
  ppu.WriteRegister(0x2000, 0xff);

  CHECK(ppu.ctrl.nametable == 0x3);
  CHECK(ppu.ctrl.addressIncrement == 32);
  CHECK(ppu.ctrl.spriteTableAddr == 0x1000);
  CHECK(ppu.ctrl.backgroundTableAddr == 0x1000);
  CHECK(ppu.ctrl.spriteHeight == 16);
  CHECK(ppu.ctrl.generateNmiAtVBlank == true);

  // Write to a mirror of 0x2000
  ppu.WriteRegister(0x3ff8, 0x01);

  CHECK(ppu.ctrl.nametable == 0x1);
  CHECK(ppu.ctrl.addressIncrement == 1);
  CHECK(ppu.ctrl.spriteTableAddr == 0x0000);
  CHECK(ppu.ctrl.backgroundTableAddr == 0x0000);
  CHECK(ppu.ctrl.spriteHeight == 8);
  CHECK(ppu.ctrl.generateNmiAtVBlank == false);
}

TEST_CASE("Ppu_RegPpumask", "[ppu]") {
  // Test various PPUCTRL register settings
  nesturbia::Cartridge cartridge;
  nesturbia::Ppu ppu(cartridge, [](uint8, uint8, uint32_t) {});

  // Write directly to 0x2001
  ppu.WriteRegister(0x2001, 0xff);

  CHECK(ppu.mask.greyscale == true);
  CHECK(ppu.mask.showBackgroundInLeftmost8Px == true);
  CHECK(ppu.mask.showSpritesInLeftmost8Px == true);
  CHECK(ppu.mask.showBackground == true);
  CHECK(ppu.mask.showSprites == true);
  CHECK(ppu.mask.emphasizeRed == true);
  CHECK(ppu.mask.emphasizeGreen == true);
  CHECK(ppu.mask.emphasizeBlue == true);

  // Write to a mirror of 0x2001
  ppu.WriteRegister(0x3ff9, 0x00);

  CHECK(ppu.mask.greyscale == false);
  CHECK(ppu.mask.showBackgroundInLeftmost8Px == false);
  CHECK(ppu.mask.showSpritesInLeftmost8Px == false);
  CHECK(ppu.mask.showBackground == false);
  CHECK(ppu.mask.showSprites == false);
  CHECK(ppu.mask.emphasizeRed == false);
  CHECK(ppu.mask.emphasizeGreen == false);
  CHECK(ppu.mask.emphasizeBlue == false);
}

TEST_CASE("Ppu_RegPpustatus", "[ppu]") {
  // Test various PPUCTRL register settings
  nesturbia::Cartridge cartridge;
  nesturbia::Ppu ppu(cartridge, [](uint8, uint8, uint32_t) {});

  ppu.status.latchedData = 0;
  ppu.status.spriteOverflow = false;
  ppu.status.sprite0Hit = false;
  ppu.status.vblankStarted = false;

  // Write to PPUSTATUS (0x2002)
  // PPUSTATUS is read-only, so its value does not change due to this write
  ppu.WriteRegister(0x2002, 0x3a);
  CHECK(ppu.status == 0);

  // The above write caused 0x3a to be latched by the PPU
  // That means that reading PPUSTATUS will have the bottom 5 bits match that latched value
  CHECK(ppu.ReadRegister(0x2002) == 0x1a);

  // Ensure that reading PPUSTATUS clears the write latch used by PPUSCROLL/PPUADDR
  ppu.writeLatch = true;
  static_cast<void>(ppu.ReadRegister(0x2002));

  CHECK(ppu.writeLatch == false);

  // Check other bits of the status register
  // Use a mirror of 0x2002 (0x3ffa)
  ppu.status.spriteOverflow = true;
  ppu.status.sprite0Hit = true;
  ppu.status.vblankStarted = true;

  CHECK(ppu.ReadRegister(0x3ffa) == 0xfa);
}

TEST_CASE("Ppu_ReadLatch", "[ppu]") {
  // Test that reading write-only registers produces the last value written
  nesturbia::Cartridge cartridge;
  nesturbia::Ppu ppu(cartridge, [](uint8, uint8, uint32_t) {});

  ppu.WriteRegister(0x2000, 0xff);
  CHECK(ppu.ReadRegister(0x2000) == 0xff);
  CHECK(ppu.ReadRegister(0x2001) == 0xff);
  CHECK(ppu.ReadRegister(0x2003) == 0xff);
  CHECK(ppu.ReadRegister(0x2005) == 0xff);
  CHECK(ppu.ReadRegister(0x2006) == 0xff);
}

TEST_CASE("Ppu_OamWrite", "[ppu]") {
  nesturbia::Cartridge cartridge;
  nesturbia::Ppu ppu(cartridge, [](uint8, uint8, uint32_t) {});

  ppu.oamaddr = 0x10;

  // Write OAM using the base register
  ppu.WriteRegister(0x2004, 0xaa);

  // Then write using mirrored addresses
  ppu.WriteRegister(0x200c, 0xbb);
  ppu.WriteRegister(0x3ffc, 0xcc);

  // Each write should increment the OAM address
  CHECK(ppu.oam[0x10] == 0xaa);
  CHECK(ppu.oam[0x11] == 0xbb);
  CHECK(ppu.oam[0x12] == 0xcc);
  CHECK(ppu.oamaddr == 0x13);
}
