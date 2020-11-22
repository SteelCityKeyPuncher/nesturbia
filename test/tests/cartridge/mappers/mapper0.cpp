#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cartridge.hpp"
using namespace nesturbia;

TEST_CASE("Nesturbia_Cartridge_Mapper0_Valid", "[mapper]") {
  std::array<uint8_t, 0xa010> rom;
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 1 * 16K
  rom[4] = 1;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  Cartridge cartridge;
  CHECK(cartridge.LoadRom(rom.data(), 16 + 0x4000 + 0x2000));

  // Change PRG-ROM to: 2 * 16K
  rom[4] = 2;

  CHECK(cartridge.LoadRom(rom.data(), rom.size()));
}

TEST_CASE("Nesturbia_Cartridge_Mapper0_InvalidPRGSize", "[mapper]") {
  std::array<uint8_t, 0xa010> rom;
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 3 * 16K
  rom[4] = 3;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  Cartridge cartridge;
  CHECK(!cartridge.LoadRom(rom.data(), rom.size()));
}

TEST_CASE("Nesturbia_Cartridge_Mapper0_InvalidCHRSize", "[mapper]") {
  std::array<uint8_t, 0xa010> rom;
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 2 * 16K
  rom[4] = 2;

  // CHR-ROM: 3 * 8K
  rom[5] = 3;

  Cartridge cartridge;
  CHECK(!cartridge.LoadRom(rom.data(), rom.size()));
}

TEST_CASE("Nesturbia_Cartridge_Mapper0_InvalidROMSize", "[mapper]") {
  // Test when the ROM file is not large enough to hold header + PRG + CHR sections
  std::array<uint8_t, 0x10000> rom;
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 2 * 16K
  rom[4] = 2;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  // Too small
  Cartridge cartridge;
  CHECK(!cartridge.LoadRom(rom.data(), 0x100));

  // Too big
  CHECK(!cartridge.LoadRom(rom.data(), 0x10000));
}
