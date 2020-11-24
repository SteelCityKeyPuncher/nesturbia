#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Power", "[apu]") {
  // Test the power-up state of the APU
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  cpu.Power();

  CHECK(cpu.frameCounter.shiftRegister == 0x7fff);
  CHECK(cpu.frameCounter.interruptInhibit == false);
  CHECK(cpu.frameCounter.mode == false);
}
