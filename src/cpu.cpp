#include <utility>

#include "nesturbia/cpu.hpp"

namespace nesturbia {
cpu::cpu(read_callback_t readCallback, write_callback_t writeCallback)
    : readCallback(std::move(readCallback)), writeCallback(std::move(writeCallback)) {}

uint8_t cpu::read(uint16_t address) {
  if (!readCallback) {
    return 0xff;
  }

  return readCallback(address);
}

uint16_t cpu::read16(uint16_t address) {
  return read(address) | (read(static_cast<uint16_t>(address + 1)) << 8);
}

void cpu::write(uint16_t address, uint8_t value) {
  if (writeCallback) {
    writeCallback(address, value);
  }
}

void cpu::write16(uint16_t address, uint16_t value) {
  writeCallback(address, static_cast<uint8_t>(value));
  writeCallback((uint16_t)(address + 1), static_cast<uint8_t>(value >> 8));
}
} // namespace nesturbia
