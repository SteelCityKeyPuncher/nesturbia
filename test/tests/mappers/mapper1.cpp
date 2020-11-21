#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/mapper.hpp"
#include "nesturbia/mappers/mapper1.hpp"
using namespace nesturbia;

TEST_CASE("Nesturbia_Mapper1_Valid", "[mapper]") {
  std::array<uint8_t, 0x20010> rom;
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 8 * 16K
  rom[4] = 8;

  // CHR-ROM: 0 * 8K
  rom[5] = 0;

  // Mapper: 1
  rom[6] |= 1U << 4;

  auto mapper = Mapper::Create(rom.data(), 16 + 0x20000);
  CHECK(mapper != nullptr);
}

TEST_CASE("Nesturbia_Mapper1_Control", "[mapper]") {
  std::array<uint8_t, 0x40010> rom;
  rom[0] = 'N';
  rom[1] = 'E';
  rom[2] = 'S';
  rom[3] = 0x1a;

  // PRG-ROM: 16 * 16K
  rom[4] = 16;

  // CHR-ROM: 0 * 8K
  rom[5] = 0;

  // Mapper: 1
  rom[6] |= 1U << 4;

  auto mapper = Mapper::Create(rom.data(), 16 + 0x40000);
  REQUIRE(mapper != nullptr);

  auto mapperInternal = reinterpret_cast<Mapper1 *>(mapper.get());
  REQUIRE(mapperInternal != nullptr);

  // Test internal control register ($8000-$9fff)
  // Reset the shift register
  mapper->Write(0x8abc, 0x80);
  CHECK(mapperInternal->shiftRegister.bit(4) == true);
  CHECK((mapperInternal->shiftRegister & 0xf) == 0);

  // Shift in 0xf
  mapper->Write(0xaaaa, 0x01);
  CHECK(mapperInternal->shiftRegister.bit(0) == false);

  mapper->Write(0xbbbb, 0x01);
  CHECK(mapperInternal->shiftRegister.bit(0) == false);

  mapper->Write(0xcccc, 0x01);
  CHECK(mapperInternal->shiftRegister.bit(0) == false);

  mapper->Write(0xdddd, 0x01);
  CHECK(mapperInternal->shiftRegister.bit(0) == true);

  // Final write (bits 14-13 should be 0b00 to write control register)
  mapper->Write(0x8000, 0x01);

  CHECK(mapperInternal->shiftRegister.bit(0) == false);
  CHECK(mapperInternal->controlRegister.mirrorType == 0x3);
  CHECK(mapperInternal->controlRegister.prgRomBankMode == 0x3);
  CHECK(mapperInternal->controlRegister.chrRomBankMode == true);

  // Test internal PRG bank register ($e000-$ffff)
  mapper->Write(0xe000, 0x80);

  // Lower 4 bits are 10 (0xa or 0b1010)
  mapper->Write(0xe000, 0x0);
  mapper->Write(0xe000, 0x1);
  mapper->Write(0xe000, 0x0);
  mapper->Write(0xe000, 0x1);

  // Upper bit is 1
  mapper->Write(0xe000, 0x1);

  CHECK(mapperInternal->prgBankRegister.prgRomBank == 0xa);
  CHECK(mapperInternal->prgBankRegister.prgRamChipEnable == true);

  // TODO make other tests:
  // * Middle two internal registers
}
