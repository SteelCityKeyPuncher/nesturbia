#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_SEI", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // SEI
  memory[0x00] = 0x78;

  cpu.Power();

  cpu.P.I = false;

  cpu.executeInstruction();

  CHECK(cpu.P.I == true);
  CHECK(cpu.cycles == 7 + 2);
}
