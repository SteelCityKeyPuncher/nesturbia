#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Noise_Registers", "[apu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  cpu.Power();

  // $400c: bit 5 is 'length counter halt' boolean
  cpu.write(0x400c, 0x20);
  CHECK(cpu.noiseChannel.length.halt == true);

  cpu.write(0x400c, 0x00);
  CHECK(cpu.noiseChannel.length.halt == false);

  // $400e bit 7 is the 'mode' of the noise channel
  cpu.write(0x400e, 0x80);
  CHECK(cpu.noiseChannel.mode == true);

  cpu.write(0x400e, 0x00);
  CHECK(cpu.noiseChannel.mode == false);

  // TODO add more tests
}
