#include <utility>

#include "nesturbia/cpu.hpp"

namespace nesturbia {
Cpu::Cpu(read_callback_t readCallback, write_callback_t writeCallback)
    : readCallback(std::move(readCallback)), writeCallback(std::move(writeCallback)) {}

void Cpu::Power() {
  A = 0x00;
  X = 0x00;
  Y = 0x00;
  S = 0xfd;
  PC = read16(0xfffc);
  P = 0x34;

  cycles = 7;
}

uint8_t Cpu::read(uint16_t address) {
  if (!readCallback) {
    return 0xff;
  }

  return readCallback(address);
}

uint16_t Cpu::read16(uint16_t address) {
  return read(address) | (read(static_cast<uint16_t>(address + 1)) << 8);
}

void Cpu::write(uint16_t address, uint8_t value) {
  if (writeCallback) {
    writeCallback(address, value);
  }
}

void Cpu::write16(uint16_t address, uint16_t value) {
  writeCallback(address, static_cast<uint8_t>(value));
  writeCallback((uint16_t)(address + 1), static_cast<uint8_t>(value >> 8));
}
} // namespace nesturbia
