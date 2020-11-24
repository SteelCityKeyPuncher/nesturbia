#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_JSR", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // JSR
  memory[0x00] = 0x20;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  CHECK(cpu.S == 0xfd);

  cpu.executeInstruction();

  CHECK(cpu.S == 0xfb);
  CHECK(memory.at(0x1fd) == 0x00);
  CHECK(memory.at(0x1fc) == 0x02);
  CHECK(cpu.PC == 0xbeef);
  CHECK(cpu.cycles == 7 + 6);
}
