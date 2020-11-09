#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Instructions_JMP_abs", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // JMP
  memory[0x00] = 0x4c;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.PC == 0xbeef);
  CHECK(cpu.cycles == 7 + 3);
}

TEST_CASE("Cpu_Instructions_JMP_ind", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write);

  // JMP
  memory[0x00] = 0x6c;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbeef] = 0xfe;
  memory[0xbef0] = 0xca;

  cpu.Power();
  cpu.executeInstruction();

  CHECK(cpu.PC == 0xcafe);
  CHECK(cpu.cycles == 7 + 5);
}
