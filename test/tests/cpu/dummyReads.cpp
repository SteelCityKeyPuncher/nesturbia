#include <array>
#include <cstdint>

#include "catch2/catch_all.hpp"

#include "nesturbia/cpu.hpp"
using namespace nesturbia;

TEST_CASE("Cpu_DummyReadsBasic", "[cpu]") {
  // These tests are based upon the cpu_dummy_reads.nes ROM created by the legendary Blargg
  // They test that the absolute,X and the indirect,Y (aka (zero-page),Y) addressing modes produce
  // a dummy read when a page boundary is crossed
  // TODO: test other possible dummy reads (although not many things rely on this behavior)

  // Memory is all zeroes to begin with
  std::array<uint8_t, 0x10000> memory;

  // The read callback is special in that it inverts the value at the given address
  // This is so that we can tell if a given memory address has been read by a dummy read more easily
  auto read = [&memory](uint16_t address) {
    const auto value = memory.at(address);
    memory.at(address) = ~value;
    return value;
  };

  auto write = [&memory](uint16_t address, uint8_t value) { memory.at(address) = value; };

  Cpu cpu(read, write, [] {});

  //
  // absolute,X addressing mode
  //

  // LDA: A = [0xbeef + X(0x11)](0xbb)
  // Page crossing = dummy read
  memory[0x00] = 0xbd;
  memory[0x01] = 0xef;
  memory[0x02] = 0xbe;

  memory[0xbf00] = 0xbb;

  cpu.Power();

  cpu.X = 0x11;

  cpu.executeInstruction();

  // 0xbeef + 0x11 causes an 8-bit carry (0xef + 0x11 > 0xff)
  // As a result, a dummy read should have occurred at the memory location before the carry occurred
  // That is: 0xef + 0x11 = 0x00 with a carry
  // 0xbe00 should have a 'dummy read' and therefore it should have a flipped value due to the read
  // callback
  CHECK(memory[0xbe00] == 0xff);

  CHECK(cpu.A == 0xbb);
  CHECK(cpu.P.Z == false);
  CHECK(cpu.P.N == true);
  CHECK(cpu.cycles == 7 + 5);

  //
  // (indirect),Y a.k.a. (zero-page),Y addressing mode
  //

  // Reset all memory just in case
  for (auto &byte : memory) {
    byte = 0;
  }

  // STA: [0xbeef + Y(0x40)] = A(0xbb)
  // Page crossing = dummy read
  memory[0x00] = 0x91;
  memory[0x01] = 0xab;

  memory[0xab] = 0xef;
  memory[0xac] = 0xbe;

  cpu.Power();

  cpu.A = 0xbb;
  cpu.Y = 0x40;

  cpu.executeInstruction();

  // 0xbeef + 0x40 = 0xbe2f (the dummy read location) but then has a carry at position 0x0100
  // Therefore, the value in memory location 0xbe2f should have flipped due to the read callback
  CHECK(memory[0xbe2f] == 0xff);

  CHECK(memory.at(0xbf2f) == 0xbb);
  CHECK(cpu.cycles == 7 + 6);
}
