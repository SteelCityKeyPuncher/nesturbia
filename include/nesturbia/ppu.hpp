#ifndef NESTURBIA_PPU_HPP_INCLUDED
#define NESTURBIA_PPU_HPP_INCLUDED

#include <cstdint>
#include <functional>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Ppu {
  // Data

  // Public functions

  // Private functions
  uint8 read(uint16 address);
  void write(uint16 address, uint8 value);
};

} // namespace nesturbia

#endif // NESTURBIA_PPU_HPP_INCLUDED
