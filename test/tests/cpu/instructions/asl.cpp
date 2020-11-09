#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_ASL_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ASL: [0xab](0xff) <<= 1 = 0xfe
  memory[0x00] = 0x06;
  memory[0x01] = 0xab;

  memory[0xab] = 0xff;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xfe);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // ASL: [0xab](0x80) <<= 1 = 0x00
  memory[0x00] = 0x06;
  memory[0x01] = 0xab;

  memory[0xab] = 0x80;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // ASL: [0xab](0x01) <<= 1 = 0x02
  memory[0x00] = 0x06;
  memory[0x01] = 0xab;

  memory[0xab] = 0x01;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x02);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_ASL_acc", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ASL: A(0xff) <<= 1 = 0xfe
  memory[0x00] = 0x0a;

  cpu.Power();

  cpu.A = 0xff;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xfe);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_ASL_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ASL: [0xbeef](0x2e) <<= 1 = 0x5c
  memory[0x00] = 0x0e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x2e;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x5c);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ASL_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ASL: [0xab + X(3)](0x2e) <<= 1 = 0x5c
  memory[0x00] = 0x16;
  memory[0x01] = 0xab;

  memory[0xae] = 0x2e;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0x5c);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ASL_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ASL: [0xbeef + X(3)](0x2e) <<= 1 = 0x5c
  memory[0x00] = 0x1e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x2e;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0x5c);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 7);
}
