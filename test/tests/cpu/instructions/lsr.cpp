#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_LSR_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LSR: [0xab](0xff) >>= 1 = 0x7f
  memory[0x00] = 0x46;
  memory[0x01] = 0xab;

  memory[0xab] = 0xff;

  cpu.Power();

  cpu.P.N = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x7f);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // LSR: [0xab](0x01) >>= 1 = 0x00
  memory[0x00] = 0x46;
  memory[0x01] = 0xab;

  memory[0xab] = 0x01;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x00);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // LSR: [0xab](0x04) >>= 1 = 0x02
  memory[0x00] = 0x46;
  memory[0x01] = 0xab;

  memory[0xab] = 0x04;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x02);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_LSR_acc", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LSR: A(0xff) >>= 1 = 0x7f
  memory[0x00] = 0x4a;

  cpu.Power();

  cpu.A = 0xff;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x7f);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_LSR_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LSR: [0xbeef](0xff) >>= 1 = 0x7f
  memory[0x00] = 0x4e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xff;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x7f);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_LSR_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LSR: [0xab + X(3)](0xff) >>= 1 = 0x7f
  memory[0x00] = 0x56;
  memory[0x01] = 0xab;

  memory[0xae] = 0xff;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0x7f);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_LSR_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // LSR: [0xbeef + X(3)](0xff) >>= 1 = 0x7f
  memory[0x00] = 0x5e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0xff;

  cpu.Power();

  cpu.X = 0x03;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0x7f);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 7);
}
