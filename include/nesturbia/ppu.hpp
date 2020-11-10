#ifndef NESTURBIA_PPU_HPP_INCLUDED
#define NESTURBIA_PPU_HPP_INCLUDED

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Ppu {
  // Data

  // Public functions
  uint8 Read(uint16 address);
  void Write(uint16 address, uint8 value);

  // Private functions
};

} // namespace nesturbia

#endif // NESTURBIA_PPU_HPP_INCLUDED
