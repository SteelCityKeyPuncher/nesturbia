#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_INY", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // INY: Y(0xaa)++ = 0xab
  memory[0x00] = 0xc8;

  cpu.Power();

  cpu.Y = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.Y == 0xab);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 2);
}
