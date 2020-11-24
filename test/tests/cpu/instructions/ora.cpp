#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_ORA_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0xaa) | [0xbeef](0x55) = 0xff
  memory[0x00] = 0x01;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // ORA: A(0x0f) | [0xbeef](0x70) = 0x7f
  memory[0x00] = 0x01;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x70;

  cpu.Power();

  cpu.A = 0x0f;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x7f);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // ORA: A(0x00) | [0xbeef](0x00) = 0x00
  memory[0x00] = 0x01;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x00;

  cpu.Power();

  cpu.A = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ORA_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0x0a) | [0xab](0xa0) = 0xaa
  memory[0x00] = 0x05;
  memory[0x01] = 0xab;

  memory[0xab] = 0xa0;

  cpu.Power();

  cpu.A = 0x0a;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_ORA_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0x0a) | imm(0xa0) = 0xaa
  memory[0x00] = 0x09;
  memory[0x01] = 0xa0;

  cpu.Power();

  cpu.A = 0x0a;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_ORA_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0x55) | [0xbeef](0xaa) = 0xff
  memory[0x00] = 0x0d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xaa;

  cpu.Power();

  cpu.A = 0x55;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_ORA_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0xb0) | [0xbeef + Y(0)](0x0b) = 0xbb
  memory[0x00] = 0x11;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x0b;

  cpu.Power();

  cpu.A = 0xb0;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // ORA: A(0xa0) | [0xbeef + Y(0x40)](0x0a) = 0xaa
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x11;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0x0a;

  cpu.Power();

  cpu.A = 0xa0;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ORA_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0xb0) | [0xab + X(3)](0x0b) = 0xbb
  memory[0x00] = 0x15;
  memory[0x01] = 0xab;

  memory[0xae] = 0x0b;

  cpu.Power();

  cpu.A = 0xb0;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_ORA_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0xf0) | [0xbeef + Y(3)](0x01) = 0xf1
  memory[0x00] = 0x19;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x01;

  cpu.Power();

  cpu.A = 0xf0;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xf1);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // ORA: A(0x0c) | [0xbeef + Y(0x11)](0x03) = 0x0f
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x19;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0x03;

  cpu.Power();

  cpu.A = 0x0c;
  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x0f);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_ORA_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ORA: A(0xf0) | [0xbeef + X(3)](0x0f) = 0xff
  memory[0x00] = 0x1d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x0f;

  cpu.Power();

  cpu.A = 0xf0;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // ORA: A(0x00) | [0xbeef + X(0x11)](0x00) = 0x00
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x1d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0x00;

  cpu.Power();

  cpu.A = 0x00;
  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}
