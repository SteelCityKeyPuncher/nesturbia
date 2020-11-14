#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_PLA", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;
  using namespace nesturbia;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // PLA
  memory[0x00] = 0x68;

  cpu.Power();

  CHECK(cpu.S == 0xfd);

  // Fake push (so that cpu.cycles is not modified)
  memory[0x1fd] = 0xff;
  cpu.S = 0xfc;

  cpu.executeInstruction();

  CHECK(cpu.A == 0xff);
  CHECK(cpu.S == 0xfd);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 4);
}
