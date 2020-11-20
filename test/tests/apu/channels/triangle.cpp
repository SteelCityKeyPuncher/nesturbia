#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/apu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Triangle_Registers", "[apu]") {
  Apu apu;

  apu.Power();

  apu.WriteRegister(0x4008, 0xff);

  CHECK(apu.triangleChannel.length.halt == true);
  CHECK(apu.triangleChannel.linearCounter.control == true);
  CHECK(apu.triangleChannel.linearCounter.load == 0x7f);

  // TODO add more tests
}
