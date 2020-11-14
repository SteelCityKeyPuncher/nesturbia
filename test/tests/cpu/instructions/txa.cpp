#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_TXA", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // TXA: A = X(0xaa)
  memory[0x00] = 0x8a;

  cpu.Power();

  cpu.X = 0xaa;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.P.N == true);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.cycles == 7 + 2);

  // TXA: A = X(0x00)
  memory[0x00] = 0x8a;

  cpu.Power();

  cpu.X = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.cycles == 7 + 2);
}
