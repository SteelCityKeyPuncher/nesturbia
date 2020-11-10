#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_CLC", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // CLC
  memory[0x00] = 0x18;

  cpu.Power();

  cpu.P.C = true;

  cpu.executeInstruction();

  CHECK(cpu.P.C == false);
  CHECK(cpu.cycles == 7 + 2);
}
