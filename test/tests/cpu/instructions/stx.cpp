#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_STX_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // STX: [0xab] = X(0xbb)
  memory[0x00] = 0x86;
  memory[0x01] = 0xab;

  cpu.Power();

  cpu.X = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xbb);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_STX_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // STX: [0xbeef] = X(0xbb)
  memory[0x00] = 0x8e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.X = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_STX_zpy", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // STX: [0xab + Y(3)] = X(0xbb)
  memory[0x00] = 0x96;
  memory[0x01] = 0xab;

  cpu.Power();

  cpu.X = 0xbb;
  cpu.Y = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);
}
