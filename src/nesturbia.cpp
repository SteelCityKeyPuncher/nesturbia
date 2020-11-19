#include "nesturbia/nesturbia.hpp"

namespace nesturbia {

Nesturbia::Nesturbia()
    : cpu([this](uint16 address) { return cpuReadCallback(address); },
          [this](uint16 address, uint8 value) { cpuWriteCallback(address, value); },
          [this] { cpuTickCallback(); }),
      ppu(cartridge, [this] { cpu.NMI(); }) {}

void Nesturbia::SetAudioSampleCallback(Apu::sample_callback_t sampleCallback, uint32_t sampleRate) {
  apu.SetSampleCallback(sampleCallback, sampleRate);
}

bool Nesturbia::LoadRom(const void *romData, size_t romDataSize) {
  if (!cartridge.LoadRom(romData, romDataSize)) {
    return false;
  }

  cpu.Power();
  apu.Power();
  ppu.Power();

  return true;
}

void Nesturbia::RunFrame(const Joypad::input_t &joypadInput1, const Joypad::input_t &joypadInput2) {
  // Update joypad inputs
  joypads[0].SetInput(joypadInput1);
  joypads[1].SetInput(joypadInput2);

  isNewFrame = false;
  for (;;) {
    cpu.executeInstruction();
    if (isNewFrame) {
      break;
    }
  }
}

uint8 Nesturbia::cpuReadCallback(uint16 address) {
  // TODO read from joypad 1 (zero-indexed, the second one)
  if (address < 0x2000) {
    // RAM (0x0000 - 0x07ff, but mirrored up to 0x1fff)
    return ram[address & 0x7ff];
  } else if (address < 0x4000) {
    // PPU registers (and their mirrors)
    return ppu.ReadRegister(address);
  } else if (address < 0x4016) {
    return apu.ReadRegister(address);
  } else if (address == 0x4016) {
    return joypads[0].Read();
  } else if (address < 0x4020) {
    return apu.ReadRegister(address);
  } else {
    return cartridge.Read(address);
  }
}

void Nesturbia::cpuWriteCallback(uint16 address, uint8 value) {
  if (address < 0x2000) {
    // RAM (0x0000 - 0x07ff, but mirrored up to 0x1fff)
    ram[address & 0x7ff] = value;
  } else if (address < 0x4000) {
    // PPU registers (and their mirrors)
    ppu.WriteRegister(address, value);
  } else if (address < 0x4014) {
    apu.WriteRegister(address, value);
  } else if (address == 0x4014) {
    for (int i = 0; i < 256; i++) {
      ppu.WriteRegister(0x2004, cpuReadCallback(value * 0x100 + i));
    }
  } else if (address == 0x4015) {
    apu.WriteRegister(address, value);
  } else if (address == 0x4016) {
    // Joypad strobes
    joypads[0].Strobe(value.bit(0));
    joypads[1].Strobe(value.bit(0));
  } else if (address < 0x4020) {
    apu.WriteRegister(address, value);
  } else {
    cartridge.Write(address, value);
  }
}

void Nesturbia::cpuTickCallback() {
  // Each CPU tick results in 3 PPU ticks
  isNewFrame = isNewFrame || ppu.Tick();
  isNewFrame = isNewFrame || ppu.Tick();
  isNewFrame = isNewFrame || ppu.Tick();

  // Each CPU tick results in 1 APU tick
  // (the APU is actually part of the CPU)
  apu.Tick();
}

} // namespace nesturbia
