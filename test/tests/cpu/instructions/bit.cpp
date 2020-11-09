#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_BIT_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // BIT (operand == 0xaa, A == 0xcc)
  memory[0x00] = 0x24;
  memory[0x01] = 0x65;

  memory[0x65] = 0xaa;

  cpu.Power();

  cpu.A = 0xcc;

  cpu.executeInstruction();

  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.V == false);
  CHECK(cpu.PC == 2);
  CHECK(cpu.cycles == 7 + 3);

  // BIT (operand == 0xff, A == 0x40)
  memory[0x00] = 0x24;
  memory[0x01] = 0x65;

  memory[0x65] = 0xff;

  cpu.Power();

  cpu.A = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.V == true);
  CHECK(cpu.PC == 2);
  CHECK(cpu.cycles == 7 + 3);

  // BIT (operand == 0x40, A == 0xf0)
  memory[0x00] = 0x24;
  memory[0x01] = 0x65;

  memory[0x65] = 0x40;

  cpu.Power();

  cpu.A = 0xf0;

  cpu.executeInstruction();

  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.V == true);
  CHECK(cpu.PC == 2);
  CHECK(cpu.cycles == 7 + 3);

  // BIT (operand == 0x0f, A == 0xf0)
  memory[0x00] = 0x24;
  memory[0x01] = 0x65;

  memory[0x65] = 0x0f;

  cpu.Power();

  cpu.A = 0xf0;

  cpu.executeInstruction();

  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.PC == 2);
  CHECK(cpu.cycles == 7 + 3);
}
