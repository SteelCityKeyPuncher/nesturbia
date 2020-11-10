#include "nesturbia/nesturbia.hpp"

namespace nesturbia {

Nesturbia::Nesturbia()
    : cpu([this](uint16 address) { return cpuReadCallback(address); },
          [this](uint16 address, uint8 value) { cpuWriteCallback(address, value); },
          [this] { cpuTickCallback(); }) {}

bool Nesturbia::LoadRom(const std::string &romPath) {
  mapper = Mapper::Create(romPath);

  return mapper != nullptr;
}

void Nesturbia::Run() {
  cpu.Power();
  cpu.executeInstruction();
  cpu.executeInstruction();
  cpu.executeInstruction();
  cpu.executeInstruction();
}

uint8 Nesturbia::cpuReadCallback(uint16 address) {
  if (address < 0x2000) {
    // RAM (0x0000 - 0x07ff, but mirrored up to 0x1fff)
    return ram[address & 0x7ff];
  } else if (address < 0x4000) {
    // PPU registers (and their mirrors)
    return ppu.Read(address);
  } else if (address >= 0x4018) {
    return mapper->Read(address);
  }

  // TODO other peripherals
  return 0;
}

void Nesturbia::cpuWriteCallback(uint16 address, uint8 value) {
  if (address < 0x2000) {
    // RAM (0x0000 - 0x07ff, but mirrored up to 0x1fff)
    ram[address & 0x7ff] = value;
  } else if (address < 0x4000) {
    // PPU registers (and their mirrors)
    ppu.Write(address, value);
  } else if (address >= 0x4018) {
    mapper->Write(address, value);
  }

  // TODO other peripherals
}

void Nesturbia::cpuTickCallback() {
  // Each CPU tick should result in 3 PPU ticks
  // TODO
}

} // namespace nesturbia
