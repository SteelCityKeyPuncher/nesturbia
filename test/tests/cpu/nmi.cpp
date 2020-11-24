#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_NMI", "[cpu]") {
  std::array<uint8_t, 0x10000> memory = {};

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  // Set NMI vector
  memory[0xfffa] = 0xef;
  memory[0xfffb] = 0xbe;

  // SED
  memory[0x00] = 0xf8;

  cpu.Power();

  CHECK(cpu.cycles == 7);
  CHECK(cpu.P == 0x4);

  cpu.NMI();
  cpu.executeInstruction();

  CHECK(cpu.S == 0xfa);
  CHECK(memory.at(0x1fd) == 0x00);
  CHECK(memory.at(0x1fc) == 0x00);
  CHECK(memory.at(0x1fb) == 0x24);
  CHECK(cpu.P.I == true);
  CHECK(cpu.PC == 0xbeef);
  CHECK(cpu.cycles == 7 + 7);
}
