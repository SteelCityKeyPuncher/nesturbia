#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/apu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_FrameCounter", "[apu]") {
  // Test the timing of the APU frame counter
  Apu apu;

  apu.Power();

  CHECK(apu.frameCounter.shiftRegister == 0x7fff);

  uint32_t tickNum = 1;

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (apu.frameCounter.shiftRegister == 0x1061) {
      break;
    }

    apu.Tick();
  }

  CHECK(tickNum == (uint32_t)(3728.5 * 2));

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (apu.frameCounter.shiftRegister == 0x3603) {
      break;
    }

    apu.Tick();
  }

  CHECK(tickNum == (uint32_t)(7456.5 * 2));

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (apu.frameCounter.shiftRegister == 0x2cd3) {
      break;
    }

    apu.Tick();
  }

  CHECK(tickNum == (uint32_t)(11185.5 * 2));

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (apu.frameCounter.shiftRegister == 0x0a1f) {
      break;
    }

    apu.Tick();
  }

  CHECK(tickNum == (uint32_t)(14914.5 * 2));

  // Check that the shift register has been reset
  apu.Tick();
  apu.Tick();

  CHECK(apu.frameCounter.shiftRegister == 0x7fff);
}
