#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/nesturbia.hpp"
using namespace nesturbia;

TEST_CASE("Nesturbia_BatteryBackedRam_Valid", "[integration]") {
  Nesturbia emulator;

  // Create a cartridge that has battery-backed RAM (i.e., save data)
  std::array<uint8_t, 0x6010> rom = {};
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 1 * 16K
  rom[4] = 1;

  // CHR-ROM: 1 * 8K
  rom[5] = 1;

  // Has battery-backed RAM
  rom[6] = 0x2;

  Cartridge cartridge;
  REQUIRE(cartridge.LoadRom(rom.data(), 16 + 0x4000 + 0x2000));

  std::array<uint8_t, 0x2000> backupRam;

  for (size_t i = 0; i < backupRam.size(); i++) {
    backupRam[i] = i;
  }

  REQUIRE(cartridge.LoadBatteryBackedRAM(backupRam.data(), backupRam.size()));

  CHECK(backupRam.at(0x1234) == 0x34);
}
