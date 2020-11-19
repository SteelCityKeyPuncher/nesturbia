#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/apu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Pulse_Registers", "[apu]") {
  Apu apu;

  apu.Power();

  apu.WriteRegister(0x4000, 0xff);

  CHECK(apu.pulseChannels[0].duty == 0x3);

  // TODO add more tests
}
