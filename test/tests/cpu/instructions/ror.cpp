#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_ROR_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ROR: [0xab](0xff) ROR 1, C(0) = 0x7f
  memory[0x00] = 0x66;
  memory[0x01] = 0xab;

  memory[0xab] = 0xff;

  cpu.Power();

  cpu.P.C = false;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x7f);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // ROR: [0xab](0x00) ROR 1, C(1) = 0x80
  memory[0x00] = 0x66;
  memory[0x01] = 0xab;

  memory[0xab] = 0x00;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x80);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // ROR: [0xab](0x01) ROR 1, C(0) = 0x00
  memory[0x00] = 0x66;
  memory[0x01] = 0xab;

  memory[0xab] = 0x01;

  cpu.Power();

  cpu.P.C = false;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_ROR_acc", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ROR: A(0xff) ROR 1, C(1) = 0xff
  memory[0x00] = 0x6a;

  cpu.Power();

  cpu.A = 0xff;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_ROR_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ROR: [0xbeef](0x2e) ROR 1, C(1) = 0x97
  memory[0x00] = 0x6e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x2e;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x97);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ROR_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ROR: [0xab + X(3)](0x2e) ROR 1, C(1) = 0x97
  memory[0x00] = 0x76;
  memory[0x01] = 0xab;

  memory[0xae] = 0x2e;

  cpu.Power();

  cpu.X = 0x03;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0x97);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ROR_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ROR: [0xbeef + X(3)](0xff) ROR 1, C(1) = 0xff
  memory[0x00] = 0x7e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xff;

  cpu.Power();

  cpu.X = 0x03;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0xff);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 7);
}
