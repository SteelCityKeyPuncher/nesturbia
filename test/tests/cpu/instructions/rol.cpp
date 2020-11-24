#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_ROL_zpg", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ROL: [0xab](0xff) ROL 1, C(0) = 0xfe
  memory[0x00] = 0x26;
  memory[0x01] = 0xab;

  memory[0xab] = 0xff;

  cpu.Power();

  cpu.P.C = false;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0xfe);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  // ROL: [0xab](0x80) ROL 1, C(1) = 0x01
  memory[0x00] = 0x26;
  memory[0x01] = 0xab;

  memory[0xab] = 0x80;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x01);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);

  // ROL: [0xab](0x01) ROL 1, C(1) = 0x03
  memory[0x00] = 0x26;
  memory[0x01] = 0xab;

  memory[0xab] = 0x01;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xab) == 0x03);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 5);
}

TEST_CASE("Cpu_Instructions_ROL_acc", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ROL: A(0xff) ROL 1, C(1) = 0xff
  memory[0x00] = 0x2a;

  cpu.Power();

  cpu.A = 0xff;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.P.C == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 2);
}

TEST_CASE("Cpu_Instructions_ROL_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ROL: [0xbeef](0x2e) ROL 1, C(1) = 0x5d
  memory[0x00] = 0x2e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0x2e;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xbeef) == 0x5d);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ROL_zpx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ROL: [0xab + X(3)](0x2e) ROL 1, C(1) = 0x5d
  memory[0x00] = 0x36;
  memory[0x01] = 0xab;

  memory[0xae] = 0x2e;

  cpu.Power();

  cpu.X = 0x03;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xae) == 0x5d);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 6);
}

TEST_CASE("Cpu_Instructions_ROL_abx", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // ROL: [0xbeef + X(3)](0x2e) ROL 1, C(1) = 0x5d
  memory[0x00] = 0x3e;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbef2] = 0x2e;

  cpu.Power();

  cpu.X = 0x03;
  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(memory.at(0xbef2) == 0x5d);
  CHECK(cpu.P.C == false);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == false);
  CHECK(cpu.cycles == 7 + 7);
}
