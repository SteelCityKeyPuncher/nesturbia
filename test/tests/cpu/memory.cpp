#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"

TEST_CASE("Cpu_Memory", "[cpu]") {
  std::array<uint8_t, 0x10000> memory;

  auto read = [&memory](uint16_t address) { return memory.at(address); };
  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  nesturbia::cpu cpu(read, write);

  // This memory is initialized to zero, so read a random address to verify this
  CHECK(read(0xffff) == 0);
  CHECK(cpu.read(0xffff) == 0);

  // Test that an 8-bit write/read produces the same value
  cpu.write(0xaaaa, 0xff);
  CHECK(memory.at(0xaaaa) == 0xff);
  CHECK(cpu.read(0xaaaa) == 0xff);

  // Test that a 16-bit write/read produces the same value
  cpu.write16(0x1234, 0xbeef);
  CHECK(memory.at(0x1234) == 0xef);
  CHECK(memory.at(0x1235) == 0xbe);
  CHECK(cpu.read16(0x1234) == 0xbeef);

  // Test that a 16-bit write/read produces the same value
  // This test writes to the end of the memory so it should wrap around
  // TODO I'm not sure if this is the correct/actual 6502 behavior
  cpu.write16(0xffff, 0xabcd);
  CHECK(memory.at(0xffff) == 0xcd);
  CHECK(memory.at(0x0000) == 0xab);
  CHECK(cpu.read16(0xffff) == 0xabcd);
}
