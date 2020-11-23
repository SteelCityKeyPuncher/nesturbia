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

  // Validate the CRC32 and MD5 hashes of this 'ROM'
  // Note: hashes don't include the 16-byte header
  CHECK(cartridge.crc32Hash == 0x6ebed2ee);
  CHECK(cartridge.md5Hash[0x0] == 0x91);
  CHECK(cartridge.md5Hash[0x1] == 0xff);
  CHECK(cartridge.md5Hash[0x2] == 0x0d);
  CHECK(cartridge.md5Hash[0x3] == 0xac);
  CHECK(cartridge.md5Hash[0x4] == 0x5d);
  CHECK(cartridge.md5Hash[0x5] == 0xf8);
  CHECK(cartridge.md5Hash[0x6] == 0x6e);
  CHECK(cartridge.md5Hash[0x7] == 0x79);
  CHECK(cartridge.md5Hash[0x8] == 0x8b);
  CHECK(cartridge.md5Hash[0x9] == 0xfe);
  CHECK(cartridge.md5Hash[0xa] == 0xf5);
  CHECK(cartridge.md5Hash[0xb] == 0xe5);
  CHECK(cartridge.md5Hash[0xc] == 0x73);
  CHECK(cartridge.md5Hash[0xd] == 0x53);
  CHECK(cartridge.md5Hash[0xe] == 0x6b);
  CHECK(cartridge.md5Hash[0xf] == 0x08);

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
