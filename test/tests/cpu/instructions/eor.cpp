#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_EOR_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) ^ [0xbeef](0xbb) = 0x11
  memory[0x00] = 0x41;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // EOR: A(0x0f) ^ [0xbeef](0xf0) = 0xff
  memory[0x00] = 0x41;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xf0;

  cpu.Power();

  cpu.A = 0x0f;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // EOR: A(0xff) ^ [0xbeef](0xff) = 0x00
  memory[0x00] = 0x41;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();

  cpu.A = 0xff;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_EOR_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) ^ [0xab](0xbb) = 0x11
  memory[0x00] = 0x45;
  memory[0x01] = 0xab;

  memory[0xab] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_EOR_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) ^ imm(0xbb) = 0x11
  memory[0x00] = 0x49;
  memory[0x01] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_EOR_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) ^ [0xbeef](0xbb) = 0x11
  memory[0x00] = 0x4d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_EOR_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) ^ [0xbeef + Y(0)](0xbb) = 0x11
  memory[0x00] = 0x51;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // EOR: A(0xaa) ^ [0xbeef + Y(0x40)](0xbb) = 0x11
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x51;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_EOR_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) + [0xab + X(3)](0xbb) = 0x11
  memory[0x00] = 0x55;
  memory[0x01] = 0xab;

  memory[0xae] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_EOR_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xaa) ^ [0xbeef + Y(3)](0xbb) = 0x11
  memory[0x00] = 0x59;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);

  // EOR: A(0xaa) ^ [0xbeef + Y(0x11)](0xbb) = 0x11
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x59;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_EOR_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // EOR: A(0xb9) ^ [0xbeef + X(3)](0x2e) = 0x11
  memory[0x00] = 0x5d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);

  // EOR: A(0xb9) ^ [0xbeef + X(0x11)](0xbb) = 0x11
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x5d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}
