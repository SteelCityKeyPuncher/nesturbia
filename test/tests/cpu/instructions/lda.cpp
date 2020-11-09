#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_LDA_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xbeef](0x11)
  memory[0x00] = 0xa1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x11;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0x11);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);

  // LDA: A = [0xbeef](0xff)
  memory[0x00] = 0xa1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);

  // LDA: A = [0xbeef](0x00)
  memory[0x00] = 0xa1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0x00;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_LDA_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xab](0xbb)
  memory[0x00] = 0xa5;
  memory[0x01] = 0xab;

  memory[0xab] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_LDA_imm", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = imm(0xbb)
  memory[0x00] = 0xa9;
  memory[0x01] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_LDA_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xbeef](0xbb)
  memory[0x00] = 0xad;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_LDA_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xbeef + Y(0)](0xbb)
  memory[0x00] = 0xb1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbeef] = 0xbb;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // LDA: A = [0xbeef + Y(0x40)](0xbb)
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xb1;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  memory[0xbf2f] = 0xbb;

  cpu.Power();

  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_LDA_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xab + X(3)](0xbb)
  memory[0x00] = 0xb5;
  memory[0x01] = 0xab;

  memory[0xae] = 0xbb;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_LDA_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xbeef + Y(3)](0xbb)
  memory[0x00] = 0xb9;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xbb;

  cpu.Power();

  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // LDA: A = [0xbeef + Y(0x11)](0xbb)
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xb9;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_LDA_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // LDA: A = [0xbeef + X(3)](0xbb)
  memory[0x00] = 0xbd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xbb;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);

  // LDA: A = [0xbeef + X(0x11)](0xbb)
  // Tests page crossing (extra cycle)
  memory[0x00] = 0xbd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);
}
