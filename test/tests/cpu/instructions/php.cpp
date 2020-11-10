#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_PHP", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // PHP (P(0x04))
  memory[0x00] = 0x08;

  cpu.Power();

  CHECK(cpu.P == 0x04);

  cpu.executeInstruction();

  CHECK(cpu.S == 0xfc);
  CHECK(memory.at(0x1fd) == 0x34);
  CHECK(cpu.cycles == 7 + 3);
}
