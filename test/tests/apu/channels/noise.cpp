#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/apu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Noise_Registers", "[apu]") {
  Apu apu;

  apu.Power();

  // $400c: bit 5 is 'length counter halt' boolean
  apu.WriteRegister(0x400c, 0x20);
  CHECK(apu.noiseChannel.length.halt == true);

  apu.WriteRegister(0x400c, 0x00);
  CHECK(apu.noiseChannel.length.halt == false);

  // $400e bit 7 is the 'mode' of the noise channel
  apu.WriteRegister(0x400e, 0x80);
  CHECK(apu.noiseChannel.envelope.loop == true);

  apu.WriteRegister(0x400e, 0x00);
  CHECK(apu.noiseChannel.envelope.loop == false);

  // TODO add more tests
}
