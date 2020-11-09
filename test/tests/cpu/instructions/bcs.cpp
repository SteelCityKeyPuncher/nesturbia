#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_BCS", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // BCS (no branch)
  memory[0x00] = 0xb0;

  cpu.Power();

  cpu.P.C = false;

  cpu.executeInstruction();

  CHECK(cpu.PC == 2);
  CHECK(cpu.cycles == 7 + 2);

  // BCS (branch same page - no extra cycles)
  memory[0x00] = 0xb0;
  memory[0x01] = 0x10;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.PC == 0x12);
  CHECK(cpu.cycles == 7 + 3);

  // BCS (branch different page - extra cycle)
  memory[0x00] = 0xb0;
  memory[0x01] = 0xfd;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.PC == 0xffff);
  CHECK(cpu.cycles == 7 + 4);
}
