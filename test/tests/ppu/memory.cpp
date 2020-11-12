#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/ppu.hpp"
using namespace nesturbia;

TEST_CASE("Ppu_Memory", "[ppu]") {
  nesturbia::Ppu ppu([](uint8, uint8, uint32_t) {});

  // Test that reading write-only registers produces the last value written
  ppu.Write(0x2000, 0xff);
  CHECK(ppu.Read(0x2000) == 0xff);
  CHECK(ppu.Read(0x2001) == 0xff);
  CHECK(ppu.Read(0x2003) == 0xff);
  CHECK(ppu.Read(0x2005) == 0xff);
  CHECK(ppu.Read(0x2006) == 0xff);
}
