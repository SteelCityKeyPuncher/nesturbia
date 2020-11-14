#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_STY_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // STY: [0xab] = Y(0xbb)
  memory[0x00] = 0x84;
  memory[0x01] = 0xab;

  cpu.Power();

  cpu.Y = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xbb);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_STY_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // STY: [0xbeef] = T(0xbb)
  memory[0x00] = 0x8c;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();

  cpu.Y = 0xbb;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);
}

TEST_CASE("Cpu_Instructions_STY_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // STY: [0xab + X(3)] = Y(0xbb)
  memory[0x00] = 0x94;
  memory[0x01] = 0xab;

  cpu.Power();

  cpu.Y = 0xbb;
  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0xbb);
  CHECK(cpu.cycles == 7 + 4);
}
