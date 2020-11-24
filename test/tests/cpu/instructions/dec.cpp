#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_DEC_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // DEC: [0xab](0xaa)-- = 0xa9
  memory[0x00] = 0xc6;
  memory[0x01] = 0xab;

  memory[0xab] = 0xaa;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xa9);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_DEC_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // DEC: [0xbeef](0x01)-- = 0x00
  memory[0x00] = 0xce;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x01;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x00);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_DEC_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // DEC: [0xab + X(3)](0x80) = 0x7f
  memory[0x00] = 0xd6;
  memory[0x01] = 0xab;

  memory[0xae] = 0x80;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0x7f);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_DEC_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // DEC: [0xbeef + X(3)](0x02)-- = 0x01
  memory[0x00] = 0xde;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x02;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0x01);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 7);
}
