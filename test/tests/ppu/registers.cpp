#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cartridge.hpp"
#include "nesturbia/ppu.hpp"
using namespace nesturbia;

TEST_CASE("Ppu_ReadLatch", "[ppu]") {
  // Test that reading write-only registers produces the last value written
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  ppu.WriteRegister(0x2000, 0xff);
  CHECK(ppu.ReadRegister(0x2000) == 0xff);
  CHECK(ppu.ReadRegister(0x2001) == 0xff);
  CHECK(ppu.ReadRegister(0x2003) == 0xff);
  CHECK(ppu.ReadRegister(0x2005) == 0xff);
  CHECK(ppu.ReadRegister(0x2006) == 0xff);
}

TEST_CASE("Ppu_RegPpuctrl", "[ppu]") {
  // Test various PPUCTRL register settings
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

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
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  // Write directly to 0x2001
  ppu.WriteRegister(0x2001, 0xff);

  CHECK(ppu.mask.grayscale == true);
  CHECK(ppu.mask.showBackgroundInLeftmost8Px == true);
  CHECK(ppu.mask.showSpritesInLeftmost8Px == true);
  CHECK(ppu.mask.showBackground == true);
  CHECK(ppu.mask.showSprites == true);
  CHECK(ppu.mask.emphasizeRed == true);
  CHECK(ppu.mask.emphasizeGreen == true);
  CHECK(ppu.mask.emphasizeBlue == true);

  // Write to a mirror of 0x2001
  ppu.WriteRegister(0x3ff9, 0x00);

  CHECK(ppu.mask.grayscale == false);
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
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

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
  ppu.addressWriteLatch = true;
  static_cast<void>(ppu.ReadRegister(0x2002));
  CHECK(ppu.addressWriteLatch == false);

  // Check other bits of the status register
  // Use a mirror of 0x2002 (0x3ffa)
  ppu.status.spriteOverflow = true;
  ppu.status.sprite0Hit = true;
  ppu.status.vblankStarted = true;

  // The first read should have an exact copy of PPUSTATUS
  // After reading, the VBLANK bit (bit 7) should be cleared
  CHECK(ppu.ReadRegister(0x3ffa) == 0xfa);

  // Test that the VBLANK bit is now cleared
  CHECK(ppu.ReadRegister(0x3ffa) == 0x7a);
}

TEST_CASE("Ppu_RegOam", "[ppu]") {
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

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

TEST_CASE("Ppu_Ppuscroll", "[ppu]") {
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  ppu.addressWriteLatch = false;

  // First write (X)
  ppu.WriteRegister(0x2005, 0xaa);

  // Coarse X is the top 5 bits (0xaa >> 3 == 0x15)
  CHECK(ppu.vramAddrLatch.fields.coarseX == 0x15);

  // Fine X is the bottom 3 bits (0xaa & 0x7 == 0x2)
  CHECK(ppu.fineX == 0x02);

  // Writing should toggle the write flag and store the previous written value
  CHECK(ppu.addressWriteLatch == true);
  CHECK(ppu.latchedValue == 0xaa);

  // Now write (Y)
  ppu.WriteRegister(0x2005, 0xbb);

  // Coarse Y is the top 5 bits (0xbb >> 3 == 0x17)
  CHECK(ppu.vramAddrLatch.fields.coarseY == 0x17);

  // Fine X is the bottom 3 bits (0xbb & 0x7 == 0x3)
  CHECK(ppu.vramAddrLatch.fields.fineY == 0x03);
}

TEST_CASE("Ppu_Ppuaddr", "[ppu]") {
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  // Power-up state
  CHECK(ppu.vramAddrLatch.value == 0);
  CHECK(ppu.vramAddr.value == 0);
  CHECK(ppu.addressWriteLatch == false);

  // Write the upper byte of PPUADDR
  ppu.WriteRegister(0x2006, 0x20);

  CHECK(ppu.vramAddrLatch.value == 0x2000);
  CHECK(ppu.vramAddr.value == 0);
  CHECK(ppu.addressWriteLatch == true);

  // Write the lower byte
  ppu.WriteRegister(0x2006, 0xff);

  CHECK(ppu.vramAddrLatch.value == 0x20ff);
  CHECK(ppu.vramAddr.value == 0x20ff);
  CHECK(ppu.addressWriteLatch == false);
}

TEST_CASE("Ppu_Ppudata_Chr", "[ppu]") {
  // Create an NROM cartridge
  std::array<uint8_t, 0xa010> rom = {};
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 1 * 16K
  rom[4] = 1;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  // Vertical mapping (bit0 == 1)
  rom[6] = 0x1;

  // Fill the CHR-ROM with sequential data
  for (int i = 0; i < 0x2000; i++) {
    rom[16 + 0x4000 + i] = static_cast<uint8>(i);
  }

  Cartridge cartridge;
  REQUIRE(cartridge.LoadRom(rom.data(), 16 + 0x4000 + 0x2000));

  Ppu ppu(cartridge, [] {});

  ppu.Power();

  // Set PPUADDR = $10ff
  ppu.WriteRegister(0x2006, 0x10);
  ppu.WriteRegister(0x2006, 0xff);

  CHECK(ppu.vramAddr.value == 0x10ff);
  CHECK(ppu.addressWriteLatch == false);

  // Read from PPUDATA once
  // Since reads from this register are buffered for lower addresses, discard this read
  static_cast<void>(ppu.ReadRegister(0x2007));

  // This read should produce the CHR-ROM value at CHR[0x1000]
  CHECK(ppu.ReadRegister(0x2007) == 0xff);
}

TEST_CASE("Ppu_Ppudata_NametableMirrorVertical", "[ppu]") {
  // Test vertical mirroring
  std::array<uint8_t, 0xa010> rom = {};
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 1 * 16K
  rom[4] = 1;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  // Vertical mapping (bit0 == 1)
  rom[6] = 0x1;

  Cartridge cartridge;
  REQUIRE(cartridge.LoadRom(rom.data(), 16 + 0x4000 + 0x2000));

  Ppu ppu(cartridge, [] {});

  ppu.Power();

  //
  // 0x37ff should map to 0x7ff
  //

  // Set PPUADDR = $37ff
  ppu.WriteRegister(0x2006, 0x37);
  ppu.WriteRegister(0x2006, 0xff);

  CHECK(ppu.vramAddr.value == 0x37ff);
  CHECK(ppu.addressWriteLatch == false);

  // Write to PPUDATA to set VRAM[map 0x37ff] = 0xaa
  ppu.WriteRegister(0x2007, 0xaa);

  CHECK(ppu.vram[0x7ff] == 0xaa);

  //
  // 0x2000 should map to 0x000
  //

  // Set PPUADDR = $2000 (beginning of VRAM mapped memory)
  ppu.WriteRegister(0x2006, 0x20);
  ppu.WriteRegister(0x2006, 0x00);

  CHECK(ppu.vramAddr.value == 0x2000);
  CHECK(ppu.addressWriteLatch == false);

  // Write to PPUDATA to set VRAM[map 0x2000] = 0xbb
  ppu.WriteRegister(0x2007, 0xbb);

  CHECK(ppu.vram[0x000] == 0xbb);
}

TEST_CASE("Ppu_Ppudata_NametableMirrorHorizontal", "[ppu]") {
  // Test horizontal mirroring
  std::array<uint8_t, 0xa010> rom = {};
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 1 * 16K
  rom[4] = 1;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  // Horizontal mapping (bit0 == 1)
  rom[6] = 0x0;

  Cartridge cartridge;
  REQUIRE(cartridge.LoadRom(rom.data(), 16 + 0x4000 + 0x2000));

  Ppu ppu(cartridge, [] {});

  ppu.Power();

  //
  // 0x3eff should map to 0x6ff
  // Vertical mirroring ties bit 11 (0x800) and to bit 11's place (0x400)
  // The rest of the bits (address & 0x3ff) stay the same
  //

  // Set PPUADDR = $3eff
  ppu.WriteRegister(0x2006, 0x3e);
  ppu.WriteRegister(0x2006, 0xff);

  CHECK(ppu.vramAddr.value == 0x3eff);
  CHECK(ppu.addressWriteLatch == false);

  // Write to PPUDATA to set VRAM[map 0x3eff] = 0xaa
  ppu.WriteRegister(0x2007, 0xaa);

  CHECK(ppu.vram[0x6ff] == 0xaa);

  //
  // 0x2000 should map to 0x000
  //

  // Set PPUADDR = $2000 (beginning of VRAM mapped memory)
  ppu.WriteRegister(0x2006, 0x20);
  ppu.WriteRegister(0x2006, 0x00);

  CHECK(ppu.vramAddr.value == 0x2000);
  CHECK(ppu.addressWriteLatch == false);

  // Write to PPUDATA to set VRAM[map 0x2000] = 0xbb
  ppu.WriteRegister(0x2007, 0xbb);

  CHECK(ppu.vram[0x000] == 0xbb);
}

TEST_CASE("Ppu_Ppudata_Palette", "[ppu]") {
  Cartridge cartridge;
  Ppu ppu(cartridge, [] {});

  ppu.Power();

  // Set PPUADDR = $3f00
  ppu.WriteRegister(0x2006, 0x3f);
  ppu.WriteRegister(0x2006, 0x00);

  CHECK(ppu.vramAddr.value == 0x3f00);
  CHECK(ppu.addressWriteLatch == false);

  // Ensure that each write increments the PPUADDR by one
  CHECK(ppu.ctrl.addressIncrement == 1);

  // Write the palette memory through PPUDATA
  // $3f00 = palette[0x00] = 0xaa (actually 0x2a because values are 6 bits)
  ppu.WriteRegister(0x2007, 0xaa);
  CHECK(ppu.paletteRam[0x00] == 0x2a);
  CHECK(ppu.vramAddr.value == 0x3f01);

  ppu.WriteRegister(0x2007, 0x3f);
  CHECK(ppu.paletteRam[0x01] == 0x3f);
  CHECK(ppu.vramAddr.value == 0x3f02);

  // Write up to $3f0f
  for (int i = 0; i < 14; i++) {
    ppu.WriteRegister(0x2007, i);
    CHECK(ppu.paletteRam[0x02 + i] == i);
    CHECK(ppu.vramAddr.value == 0x3f03 + i);
  }

  // $3f10 is a mirror of $3f00
  // Therefore palette[0x10] should not be affected (it's unused)
  ppu.WriteRegister(0x2007, 0x3f);
  CHECK(ppu.paletteRam[0x00] == 0x3f);
  CHECK(ppu.vramAddr.value == 0x3f11);
  CHECK(ppu.paletteRam[0x10] == 0);

  // Check that every 4th value is mirrored down
  for (int i = 0; i < 15; i++) {
    ppu.WriteRegister(0x2007, i + 1);

    if (((i + 1) % 4) == 0) {
      // Mirrored
      CHECK(ppu.paletteRam[0x1 + i] == (i + 1));
    } else {
      // Not mirrored
      CHECK(ppu.paletteRam[0x11 + i] == (i + 1));
    }

    CHECK(ppu.vramAddr.value == 0x3f12 + i);
  }
}
