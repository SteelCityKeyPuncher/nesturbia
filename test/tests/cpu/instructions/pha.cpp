#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_PHA", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // PHA (A(0xbc))
  memory[0x00] = 0x48;

  cpu.Power();

  CHECK(cpu.S == 0xfd);
  cpu.A = 0x48;

  cpu.executeInstruction();

  CHECK(cpu.S == 0xfc);
  CHECK(memory.at(0x1fd) == 0x48);
  CHECK(cpu.cycles == 7 + 3);
}
