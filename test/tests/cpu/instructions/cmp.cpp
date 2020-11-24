#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_CMP_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x00) - [0xbeef](0x01) = 0xff
  memory[0x00] = 0xc1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x01;

  cpu.Power();

  cpu.A = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_CMP_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0xaa) - [0xab](0xaa) = 0x00
  memory[0x00] = 0xc5;
  memory[0x01] = 0xab;

  memory[0xab] = 0xaa;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.P.C == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_CMP_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x04) - imm(0x03) = 0x28
  memory[0x00] = 0xc9;
  memory[0x01] = 0x03;

  cpu.Power();

  cpu.A = 0x04;

  cpu.executeInstruction();

  CHECK(cpu.P.C == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_CMP_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x00) - [0xbeef](0x01) = 0xff
  memory[0x00] = 0xcd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x01;

  cpu.Power();

  cpu.A = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_CMP_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x00) - [0xbeef + Y(0)](0x01) = 0xff
  memory[0x00] = 0xd1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x01;

  cpu.Power();

  cpu.A = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 5);

  // CMP: A(0x00) - [0xbeef + Y(0x40)](0x01) = 0xff
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xd1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0x01;

  cpu.Power();

  cpu.A = 0x00;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_CMP_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x00) - [0xab + X(3)](0x01) = 0xff
  memory[0x00] = 0xd5;
  memory[0x01] = 0xab;

  memory[0xae] = 0x01;

  cpu.Power();

  cpu.A = 0x00;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_CMP_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x00) - [0xbeef + Y(3)](0x01) = 0xff
  memory[0x00] = 0xd9;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x01;

  cpu.Power();

  cpu.A = 0x00;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 4);

  // CMP: A(0x00) - [0xbeef + Y(0x11)](0x01) = 0xff
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xd9;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0x01;

  cpu.Power();

  cpu.A = 0x00;
  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_CMP_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // CMP: A(0x00) - [0xbeef + X(3)](0x01) = 0xff
  memory[0x00] = 0xdd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x01;

  cpu.Power();

  cpu.A = 0x00;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 4);

  // CMP: A(0x00) - [0xbeef + X(0x11)](0x01) = 0xff
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xdd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0x01;

  cpu.Power();

  cpu.A = 0x00;
  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 5);
}
