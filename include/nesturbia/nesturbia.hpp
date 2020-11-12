#ifndef NESTURBIA_NESTURBIA_HPP_INCLUDED
#define NESTURBIA_NESTURBIA_HPP_INCLUDED

#include <array>
#include <string>

#include "nesturbia/apu.hpp"
#include "nesturbia/cpu.hpp"
#include "nesturbia/mapper.hpp"
#include "nesturbia/ppu.hpp"
#include "nesturbia/types.hpp"

namespace nesturbia {

struct Nesturbia {
  // Data
  std::array<uint8, 0x800> ram;
  Cpu cpu;
  Ppu ppu;
  Apu apu;
  Mapper::ptr_t mapper;

  std::array<uint8, 256 * 240 * 3> pixels;
  bool isNewFrame;

  // Public functions
  Nesturbia();
  bool LoadRom(const void *romData, size_t romDataSize);
  void RunFrame();

  // Private functions
  uint8 cpuReadCallback(uint16 address);
  void cpuWriteCallback(uint16 address, uint8 value);
  void cpuTickCallback();

  void setPixelCallback(uint8 x, uint8 y, uint32_t color);
};

} // namespace nesturbia

#endif // NESTURBIA_NESTURBIA_HPP_INCLUDED
