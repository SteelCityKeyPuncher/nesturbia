#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_SEC", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // SEC
  memory[0x00] = 0x38;

  cpu.Power();

  cpu.P.C = false;

  cpu.executeInstruction();

  CHECK(cpu.P.C == true);
  CHECK(cpu.cycles == 7 + 2);
}
