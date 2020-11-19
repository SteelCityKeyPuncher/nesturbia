#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/apu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Power", "[apu]") {
  // Test the power-up state of the APU
  Apu apu;

  apu.Power();

  CHECK(apu.frameCounter.shiftRegister == 0x7fff);
  CHECK(apu.frameCounter.interruptInhibit == false);
  CHECK(apu.frameCounter.mode == false);
}
