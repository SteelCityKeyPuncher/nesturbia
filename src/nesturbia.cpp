#include "nesturbia/nesturbia.hpp"

namespace nesturbia {

Nesturbia::Nesturbia()
    : cpu([this](uint16 address) { return cpuReadCallback(address); },
          [this](uint16 address, uint8 value) { cpuWriteCallback(address, value); }) {}

uint8 Nesturbia::cpuReadCallback(uint16) { return 0; }

void Nesturbia::cpuWriteCallback(uint16, uint8) {}

} // namespace nesturbia
