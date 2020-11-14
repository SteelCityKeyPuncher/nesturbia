#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_CPY_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CPY: Y(0x04) - imm(0x03) = 0x28
  memory[0x00] = 0xc0;
  memory[0x01] = 0x03;

  cpu.Power();

  cpu.Y = 0x04;

  cpu.executeInstruction();

  CHECK(cpu.P.C == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_CPY_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CPY: Y(0xaa) - [0xab](0xaa) = 0x00
  memory[0x00] = 0xc4;
  memory[0x01] = 0xab;

  memory[0xab] = 0xaa;

  cpu.Power();

  cpu.Y = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.P.C == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_CPY_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CPY: Y(0x00) - [0xbeef](0x01) = 0xff
  memory[0x00] = 0xcc;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x01;

  cpu.Power();

  cpu.Y = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 4);
}
