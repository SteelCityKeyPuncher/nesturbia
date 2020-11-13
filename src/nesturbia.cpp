#include "nesturbia/nesturbia.hpp"

namespace nesturbia {

Nesturbia::Nesturbia()
    : cpu([this](uint16 address) { return cpuReadCallback(address); },
          [this](uint16 address, uint8 value) { cpuWriteCallback(address, value); },
          [this] { cpuTickCallback(); }),
      ppu([this](uint16 address) { return readChrCallback(address); },
          [this](uint16 address, uint8 value) { writeChrCallback(address, value); },
          [this](uint8 x, uint8 y, uint32_t color) { setPixelCallback(x, y, color); }) {}

bool Nesturbia::LoadRom(const void *romData, size_t romDataSize) {
  mapper = Mapper::Create(romData, romDataSize);

  cpu.Power();

  return mapper != nullptr;
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
    ppu.WriteRegister(address, value);
  } else if (address >= 0x4018) {
    mapper->Write(address, value);
  }

  // TODO other peripherals
}

void Nesturbia::cpuTickCallback() {
  // Each CPU tick should result in 3 PPU ticks
  isNewFrame = isNewFrame || ppu.Tick();
  isNewFrame = isNewFrame || ppu.Tick();
  isNewFrame = isNewFrame || ppu.Tick();
}

uint8 Nesturbia::readChrCallback(uint16 address) {
  if (mapper) {
    return mapper->ReadChr(address);
  }

  return 0;
}

void Nesturbia::writeChrCallback(uint16 address, uint8 value) {
  if (mapper) {
    mapper->WriteChr(address, value);
  }
}

void Nesturbia::setPixelCallback(uint8 x, uint8 y, uint32_t color) {
  // TODO bounds check
  uint8 *pixel = &pixels[(y * 256 + x) * 3];

  *(pixel++) = (color >> 16) & 0xff;
  *(pixel++) = (color >> 8) & 0xff;
  *pixel++ = color & 0xff;
}

} // namespace nesturbia
