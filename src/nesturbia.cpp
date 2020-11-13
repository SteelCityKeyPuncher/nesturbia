#include "nesturbia/nesturbia.hpp"

namespace nesturbia {

Nesturbia::Nesturbia()
    : cpu([this](uint16 address) { return cpuReadCallback(address); },
          [this](uint16 address, uint8 value) { cpuWriteCallback(address, value); },
          [this] { cpuTickCallback(); }),
      ppu(cartridge) {}

bool Nesturbia::LoadRom(const void *romData, size_t romDataSize) {
  if (!cartridge.LoadRom(romData, romDataSize)) {
    return false;
  }

  cpu.Power();
  return true;
}

void Nesturbia::RunFrame() {
  isNewFrame = false;
  for (;;) {
    cpu.executeInstruction();
    if (isNewFrame) {
      break;
    }
  }
}

uint8 Nesturbia::cpuReadCallback(uint16 address) {
  if (address < 0x2000) {
    // RAM (0x0000 - 0x07ff, but mirrored up to 0x1fff)
    return ram[address & 0x7ff];
  } else if (address < 0x4000) {
    // PPU registers (and their mirrors)
    return ppu.ReadRegister(address);
  } else if (address >= 0x4018) {
    return cartridge.Read(address);
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
    ppu.WriteRegister(address, value);
  } else if (address >= 0x4018) {
    cartridge.Write(address, value);
  }

  // TODO other peripherals
}

void Nesturbia::cpuTickCallback() {
  // Each CPU tick should result in 3 PPU ticks
  isNewFrame = isNewFrame || ppu.Tick();
  isNewFrame = isNewFrame || ppu.Tick();
  isNewFrame = isNewFrame || ppu.Tick();
}

} // namespace nesturbia
