#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_ADC_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xbeef](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x61;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // ADC: A(0x00) + [0xbeef](0xbb) + C(1) = 0xbc
  memory[0x00] = 0x61;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbc);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // ADC: A(0x00) + [0xbeef](0xff) + C(1) = 0x00
  memory[0x00] = 0x61;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // ADC: A(0x7f) + [0xbeef](0x00) + C(1) = 0x80
  memory[0x00] = 0x61;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x00;

  cpu.Power();

  cpu.A = 0x7f;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x80);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == true);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // ADC: A(0xff) + [0x61aa](0xff) + C(1) = 0xff
  memory[0x00] = 0x61;
  memory[0x01] = 0xff;

  memory[0xff] = 0xaa;

  memory[0x61aa] = 0xff;

  cpu.Power();

  cpu.A = 0xff;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ADC_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xab](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x65;
  memory[0x01] = 0xab;

  memory[0xab] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 3);

  // ADC: A(0xaa) + [0xab](0x55) + C(0) = 0xff
  memory[0x00] = 0x65;
  memory[0x01] = 0xab;

  memory[0xab] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 3);

  // ADC: A(0xaa) + [0xab](0x55) + C(1) = 0x00
  memory[0x00] = 0x65;
  memory[0x01] = 0xab;

  memory[0xab] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_ADC_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + imm(0xbb) + C(0) = 0xbb
  memory[0x00] = 0x69;
  memory[0x01] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);

  // ADC: A(0xaa) + imm(0x55) + C(0) = 0xff
  memory[0x00] = 0x69;
  memory[0x01] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);

  // ADC: A(0xaa) + imm(0x55) + C(1) = 0x00
  memory[0x00] = 0x69;
  memory[0x01] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_ADC_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xbeef](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x6d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // ADC: A(0xaa) + [0xbeef](0x55) + C(0) = 0xff
  memory[0x00] = 0x6d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // ADC: A(0xaa) + [0xbeef](0x55) + C(1) = 0x00
  memory[0x00] = 0x6d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x55;

  cpu.Power();

  cpu.A = 0xaa;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_ADC_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xbeef + Y(0)](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x71;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // ADC: A(0x00) + [0xbeef + Y(0x10)](0xbb) + C(1) = 0xbc
  memory[0x00] = 0x71;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeff] = 0xbb;

  cpu.Power();

  cpu.Y = 0x10;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbc);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // ADC: A(0x00) + [0xbeef + Y(0x40)](0xbb) + C(0) = 0xbb
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x71;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0xbb;

  cpu.Power();

  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // ADC: A(0x00) + [0x0000 + Y(0xf8 = -8)](0xbb) + C(0) = 0xbb
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x71;
  memory[0x01] = 0xab;

  memory[0xab] = 0x00;
  memory[0xac] = 0x00;

  memory[0xfff8] = 0xbb;

  cpu.Power();

  cpu.Y = 0xf8;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ADC_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xab + X(3)](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x75;
  memory[0x01] = 0xab;

  memory[0xae] = 0xbb;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_ADC_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xbeef + Y(3)](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x79;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xbb;

  cpu.Power();

  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // ADC: A(0x00) + [0xbeef + Y(0x11)](0xbb) + C(0) = 0xbb
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x79;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_ADC_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // ADC: A(0x00) + [0xbeef + X(3)](0xbb) + C(0) = 0xbb
  memory[0x00] = 0x7d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xbb;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // ADC: A(0x00) + [0xbeef + X(0x11)](0xbb) + C(0) = 0xbb
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x7d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.V == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);
}
