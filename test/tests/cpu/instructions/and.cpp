#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_AND_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xaa) & [0xbeef](0xbb) = 0xaa
  memory[0x00] = 0x21;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // AND: A(0xf) & [0xbeef](0xbb) = 0x0b
  memory[0x00] = 0x21;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();

  cpu.A = 0x0f;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x0b);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // AND: A(0x00) & [0xbeef](0xff) = 0x00
  memory[0x00] = 0x21;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();

  cpu.A = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_AND_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xaa) & [0xab](0xff) = 0xaa
  memory[0x00] = 0x25;
  memory[0x01] = 0xab;

  memory[0xab] = 0xff;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_AND_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xb9) & imm(0x2e) = 0x28
  memory[0x00] = 0x29;
  memory[0x01] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_AND_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xb9) & [0xbeef](0x2e) = 0x28
  memory[0x00] = 0x2d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_AND_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xb9) & [0xbeef + Y(0)](0x2e) = 0x28
  memory[0x00] = 0x31;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // AND: A(0xb9) & [0xbeef + Y(0x40)](0x2e) = 0x28
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x31;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_AND_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xb9) + [0xab + X(3)](0x2e) = 0x28
  memory[0x00] = 0x35;
  memory[0x01] = 0xab;

  memory[0xae] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_AND_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xb9) & [0xbeef + Y(3)](0x2e) = 0xbb
  memory[0x00] = 0x39;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);

  // AND: A(0xb9) & [0xbeef + Y(0x11)](0xf0) = 0x2e
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x39;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xf0;

  cpu.Power();

  cpu.A = 0xb9;
  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xb0);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_AND_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // AND: A(0xb9) & [0xbeef + X(3)](0x2e) = 0x2e
  memory[0x00] = 0x3d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);

  // AND: A(0xb9) & [0xbeef + X(0x11)](0x2e) = 0x2e
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x3d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0x2e;

  cpu.Power();

  cpu.A = 0xb9;
  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x28);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}
