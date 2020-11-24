#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

// TODO: differentiate between 'soft reset' and 'hard reset?'
TEST_CASE("Cpu_Reset", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // Put random values into the stack to ensure that it isn't overwritten by a reset
  memory[0x1fd] = 0xaa;
  memory[0x1fc] = 0xbb;
  memory[0x1fb] = 0xcc;

  cpu.Power();

  // Store the value of S before the reset
  // Resets don't write to the stack
  // Store the stack that would normally be overwritten
  CHECK(cpu.S == 0xfd);

  // Check that A/X/Y registers don't change during a hard reset
  cpu.A = 0xaa;
  cpu.X = 0xbb;
  cpu.Y = 0xcc;

  // Check that the I (interrupt disable) flag becomes set
  cpu.P.I = false;

  cpu.Reset();

  CHECK(cpu.A == 0xaa);
  CHECK(cpu.X == 0xbb);
  CHECK(cpu.Y == 0xcc);

  CHECK(cpu.P.I == true);

  // Check that the stack hasn't been modified, only that S has decremented by 3
  CHECK(cpu.S == 0xfa);
  CHECK(memory[0x1fd] == 0xaa);
  CHECK(memory[0x1fc] == 0xbb);
  CHECK(memory[0x1fb] == 0xcc);

  // TODO: verify that the PC == the reset vector from the ROM
}
