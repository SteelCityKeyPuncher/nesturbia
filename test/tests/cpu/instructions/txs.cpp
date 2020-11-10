#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_TXS", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // TXS: S = X(0xaa)
  memory[0x00] = 0x9a;

  cpu.Power();

  cpu.X = 0xaa;
  cpu.P.N = false;
  cpu.P.Z = true;

  cpu.executeInstruction();

  CHECK(cpu.S == 0xaa);
  CHECK(cpu.P.N == false);
  CHECK(cpu.P.Z == true);
  CHECK(cpu.cycles == 7 + 2);

  // TXS: S = X(0x00)
  memory[0x00] = 0x9a;

  cpu.Power();

  cpu.X = 0x00;

  cpu.executeInstruction();

  CHECK(cpu.S == 0x00);
  CHECK(cpu.cycles == 7 + 2);
}
