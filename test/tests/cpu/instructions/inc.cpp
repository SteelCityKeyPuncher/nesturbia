#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_INC_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // INC: [0xab](0xaa)++ = 0xab
  memory[0x00] = 0xe6;
  memory[0x01] = 0xab;

  memory[0xab] = 0xaa;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xab);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_INC_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // INC: [0xbeef](0xff)++ = 0x00
  memory[0x00] = 0xee;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x00);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_INC_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // INC: [0xab + X(3)](0x7e) = 0x7f
  memory[0x00] = 0xf6;
  memory[0x01] = 0xab;

  memory[0xae] = 0x7e;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0x7f);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_INC_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // INC: [0xbeef + X(3)](0x00)++ = 0x01
  memory[0x00] = 0xfe;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x00;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0x01);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 7);
}
