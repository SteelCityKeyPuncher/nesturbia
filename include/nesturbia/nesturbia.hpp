#ifndef NESTURBIA_NESTURBIA_HPP_INCLUDED
#define NESTURBIA_NESTURBIA_HPP_INCLUDED

#include <array>
#include <string>

#include "nesturbia/apu.hpp"
#include "nesturbia/cartridge.hpp"
#include "nesturbia/cpu.hpp"
#include "nesturbia/joypad.hpp"
#include "nesturbia/mapper.hpp"
#include "nesturbia/ppu.hpp"
#include "nesturbia/types.hpp"

namespace nesturbia {

struct Nesturbia {
  // Data
  Cartridge cartridge;
  Cpu cpu;
  Ppu ppu;
  Apu apu;
  std::array<Joypad, 2> joypads;
  std::array<uint8, 0x800> ram;

  bool isNewFrame;

  // Public functions
  Nesturbia();
  void SetAudioSampleCallback(Apu::sample_callback_t sampleCallback, uint32_t sampleRate);
  bool LoadRom(const void *romData, size_t romDataSize);
  void RunFrame(const Joypad::input_t &joypadInput1 = {}, const Joypad::input_t &joypadInput2 = {});

  // Private functions
  uint8 cpuReadCallback(uint16 address);
  void cpuWriteCallback(uint16 address, uint8 value);
  void cpuTickCallback();
};

} // namespace nesturbia

#endif // NESTURBIA_NESTURBIA_HPP_INCLUDED
