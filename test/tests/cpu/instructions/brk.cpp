#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_BRK", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // BRK
  memory[0x00] = 0x00;

  // Unused byte
  memory[0x01] = 0x75;

  memory[0xfffe] = 0xef;
  memory[0xffff] = 0xbe;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.S == 0xfa);
  CHECK(memory.at(0x1fd) == 0x00);
  CHECK(memory.at(0x1fc) == 0x02);
  CHECK(memory.at(0x1fb) == 0x34);
  CHECK(cpu.P.I == true);
  CHECK(cpu.PC == 0xbeef);
  CHECK(cpu.cycles == 7 + 7);
}
