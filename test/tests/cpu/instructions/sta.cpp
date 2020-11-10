#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_STA_inx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xbeef] = A(0x11)
  memory[0x00] = 0x81;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  cpu.Power();

  cpu.A = 0x11;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x11);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_STA_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xab] = A(0xbb)
  memory[0x00] = 0x85;
  memory[0x01] = 0xab;

  cpu.Power();

  cpu.A = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xbb);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_STA_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xbeef] = A(0xbb)
  memory[0x00] = 0x8d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_STA_iny", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xbeef + Y(0)] = A(0xbb)
  memory[0x00] = 0x91;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0xbb);
  CHECK(cpu.cycles == 7 + 5);

  // STA: [0xbeef + Y(0x40)] = A(0xbb)
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x91;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  CHECK(memory.at(0xbf2f) == 0xbb);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_STA_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xab + X(3)] = A(0xbb)
  memory[0x00] = 0x95;
  memory[0x01] = 0xab;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_STA_aby", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xbeef + Y(3)] = A(0xbb)
  memory[0x00] = 0x99;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);

  // STA: [0xbeef + Y(0x11)] = A(0xbb)
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x99;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.Y = 0x11;

  cpu.executeInstruction();

  CHECK(memory.at(0xbf00) == 0xbb);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_STA_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // STA: [0xbeef + X(3)] = A(0xbb)
  memory[0x00] = 0x9d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);

  // STA: [0xbeef + X(0x11)] = A(0xbb)
  // Tests page crossing (extra cycle)
  memory[0x00] = 0x9d;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.X = 0x11;

  cpu.executeInstruction();

  CHECK(memory.at(0xbf00) == 0xbb);
  CHECK(cpu.cycles == 7 + 5);
}
