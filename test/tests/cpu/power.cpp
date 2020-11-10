#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Power", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::Cpu cpu(read, write, [] {});

  // Write '0xbeef' to the reset vector
  // The PC should be '0xbeef' after power up
  cpu.write16(0xfffc, 0xbeef);

  cpu.Power();

  CHECK(cpu.A == 0x00);
  CHECK(cpu.X == 0x00);
  CHECK(cpu.Y == 0x00);
  CHECK(cpu.S == 0xfd);
  CHECK(cpu.PC == 0xbeef);

  CHECK(cpu.P == 0x04);
  CHECK(cpu.P.I == true);

  CHECK(cpu.cycles == 7);
}
