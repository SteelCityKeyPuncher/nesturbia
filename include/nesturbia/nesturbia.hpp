#ifndef NESTURBIA_NESTURBIA_HPP_INCLUDED
#define NESTURBIA_NESTURBIA_HPP_INCLUDED

#include "nesturbia/cpu.hpp"
#include "nesturbia/types.hpp"

namespace nesturbia {

class Nesturbia {
public:
  Nesturbia();

private:
  uint8 cpuReadCallback(uint16 address);
  void cpuWriteCallback(uint16 address, uint8 value);

private:
  Cpu cpu;
};

} // namespace nesturbia

#endif // NESTURBIA_NESTURBIA_HPP_INCLUDED
