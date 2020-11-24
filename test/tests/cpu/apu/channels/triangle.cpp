#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Apu_Triangle_Registers", "[apu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  cpu.Power();

  cpu.write(0x4008, 0xff);

  CHECK(cpu.triangleChannel.length.halt == true);
  CHECK(cpu.triangleChannel.linearCounter.control == true);
  CHECK(cpu.triangleChannel.linearCounter.load == 0x7f);

  // TODO add more tests
}
