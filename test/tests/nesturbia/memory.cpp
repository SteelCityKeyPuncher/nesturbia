#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/nesturbia.hpp"
using namespace nesturbia;

TEST_CASE("Nesturbia_Memory", "[integration]") {
  Nesturbia emulator;

  // Test RAM mirroring
  emulator.cpuWriteCallback(0x1fff, 0xaa);
  CHECK(emulator.cpuReadCallback(0x07ff) == 0xaa);
  CHECK(emulator.cpuReadCallback(0x0fff) == 0xaa);
  CHECK(emulator.cpuReadCallback(0x17ff) == 0xaa);
  CHECK(emulator.cpuReadCallback(0x1fff) == 0xaa);
}
