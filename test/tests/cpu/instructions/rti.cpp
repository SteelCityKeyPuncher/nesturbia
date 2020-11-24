#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_Instructions_RTI", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // RTI
  memory[0x00] = 0x40;

  cpu.Power();

  CHECK(cpu.S == 0xfd);

  // Fake pushes (so that cpu.cycles is not modified)
  memory[0x1fd] = 0xbe;
  memory[0x1fc] = 0xef;
  memory[0x1fb] = 0xff;
  cpu.S = 0xfa;

  cpu.executeInstruction();

  CHECK(cpu.P == 0xcf);
  CHECK(cpu.PC == 0xbeef);
  CHECK(cpu.S == 0xfd);
  CHECK(cpu.cycles == 7 + 6);
}
