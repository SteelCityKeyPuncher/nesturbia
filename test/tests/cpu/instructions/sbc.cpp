#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_SBC_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xbeef](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xe1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // SBC: A(0x50) - [0xbeef](0xb0) - !C(0) = 0x9f
  memory[0x00] = 0xe1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xb0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = false;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x9f);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == true);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // SBC: A(0x50) - [0xbeef](0x30) - !C(1) = 0x20
  memory[0x00] = 0xe1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x30;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x20);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // SBC: A(0x50) - [0xbeef](0x50) - !C(1) = 0x00
  memory[0x00] = 0xe1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x50;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // SBC: A(0x50) - [0xbeef](0x60) - !C(1) = 0xf0
  memory[0x00] = 0xe1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x60;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xf0);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_SBC_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xab](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xe5;
  memory[0x01] = 0xab;

  memory[0xab] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_SBC_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - imm(0xf0) - !C(1) = 0x60
  memory[0x00] = 0xe9;
  memory[0x01] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_SBC_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xbeef](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xed;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_SBC_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xbeef + Y(0)](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xf1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // SBC: A(0x50) - [0xbeef + Y(0x10)](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xf1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeff] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.Y = 0x10;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // SBC: A(0x50) - [0xbeef + Y(0x40)](0xf0) - !C(1) = 0x60
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xf1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // SBC: A(0x50) - [0x0000 + Y(0xf8 = -8)](0xf0) - !C(1) = 0x60
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xf1;
  memory[0x01] = 0xab;

  memory[0xab] = 0x00;
  memory[0xac] = 0x00;

  memory[0xfff8] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.Y = 0xf8;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_SBC_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xab + X(3)](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xf5;
  memory[0x01] = 0xab;

  memory[0xae] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_SBC_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xbeef + Y(3)](0xf0) - !C(1) = 0x60
  memory[0x00] = 0xf9;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);

  // SBC: A(0x50) - [0xbeef + Y(0x11)](0xf0) - !C(1) = 0x60
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xf9;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_SBC_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // SBC: A(0x50) - [0xbeef + X(3)](0xf0) - !C(0) = 0x60
  memory[0x00] = 0xfd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);

  // SBC: A(0x50) - [0xbeef + X(0x11)](0xf0) - !C(0) = 0x60
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xfd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xf0;

  cpu.Power();

  cpu.A = 0x50;
  cpu.P.C = true;
  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x60);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}
