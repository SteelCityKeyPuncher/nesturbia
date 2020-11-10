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

  // Public functions
  Nesturbia();
  bool LoadRom(const std::string &romPath);
  void Run();

  // Private functions
  uint8 cpuReadCallback(uint16 address);
  void cpuWriteCallback(uint16 address, uint8 value);
};

} // namespace nesturbia

#endif // NESTURBIA_NESTURBIA_HPP_INCLUDED
