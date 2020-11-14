#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_LDY_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LDY: Y = imm(0xbb)
  memory[0x00] = 0xa0;
  memory[0x01] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.Y == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_LDY_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LDY: Y = [0xab](0xff)
  memory[0x00] = 0xa4;
  memory[0x01] = 0xab;

  memory[0xab] = 0xff;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.Y == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 3);

  // LDY: Y = [0xab](0x01)
  memory[0x00] = 0xa4;
  memory[0x01] = 0xab;

  memory[0xab] = 0x00;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.Y == 0x00);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 3);

  // LDY: Y = [0xab](0x04)
  memory[0x00] = 0xa4;
  memory[0x01] = 0xab;

  memory[0xab] = 0x04;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.Y == 0x04);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_LDY_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LDY: Y = [0xbeef](0xff)
  memory[0x00] = 0xac;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.Y == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_LDY_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LDY: Y = [0xab + X(3)](0xff)
  memory[0x00] = 0xb4;
  memory[0x01] = 0xab;

  memory[0xae] = 0xff;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.Y == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_LDY_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LDY: Y = [0xbeef + X(3)](0xff)
  // No page cross / no extra cycle
  memory[0x00] = 0xbc;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xff;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.Y == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // LDY: Y = [0xbeef + X(0x11)](0xff)
  // Page cross / extra cycle
  memory[0x00] = 0xbc;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xff;

  cpu.Power();

  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.Y == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);
}
