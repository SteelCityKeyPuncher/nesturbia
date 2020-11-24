#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("cpu_FrameCounter", "[cpu]") {
  // Test the timing of the cpu frame counter
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  cpu.Power();

  CHECK(cpu.frameCounter.shiftRegister == 0x7fff);

  uint32_t tickNum = 1;

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (cpu.frameCounter.shiftRegister == 0x1061) {
      break;
    }

    cpu.tick();
  }

  CHECK(tickNum == (uint32_t)(3728.5 * 2));

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (cpu.frameCounter.shiftRegister == 0x3603) {
      break;
    }

    cpu.tick();
  }

  CHECK(tickNum == (uint32_t)(7456.5 * 2));

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (cpu.frameCounter.shiftRegister == 0x2cd3) {
      break;
    }

    cpu.tick();
  }

  CHECK(tickNum == (uint32_t)(11185.5 * 2));

  // Check that the first quarter frame occurs at the correct time
  // Run 100K iterations in case the condition never becomes true
  for (; tickNum < 100000; tickNum++) {
    if (cpu.frameCounter.shiftRegister == 0x0a1f) {
      break;
    }

    cpu.tick();
  }

  CHECK(tickNum == (uint32_t)(14914.5 * 2));

  // Check that the shift register has been reset
  cpu.tick();
  cpu.tick();

  CHECK(cpu.frameCounter.shiftRegister == 0x7fff);
}
