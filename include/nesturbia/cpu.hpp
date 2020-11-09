#ifndef NESTURBIA_CPU_HPP_INCLUDED
#define NESTURBIA_CPU_HPP_INCLUDED

#include <cstdint>
#include <functional>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Cpu {
  // Types
  struct flags_t {
    bool C = false;
    bool Z = false;
    bool I = false;
    bool D = false;
    bool V = false;
    bool N = false;

    auto &operator=(uint8 value) {
      C = value.bit(0);
      Z = value.bit(1);
      I = value.bit(2);
      D = value.bit(3);
      V = value.bit(6);
      N = value.bit(7);

      return *this;
    }

    operator unsigned() const { return C << 0 | Z << 1 | I << 2 | D << 3 | V << 6 | N << 7; }
  };

  using read_callback_t = std::function<uint8(uint16)>;
  using write_callback_t = std::function<void(uint16, uint8)>;

  // Data
  uint8 A;
  uint8 X;
  uint8 Y;
  uint8 S;
  uint16 PC;
  flags_t P;

  read_callback_t readCallback;
  write_callback_t writeCallback;

  uint32_t cycles;

  // Public functions
  Cpu(read_callback_t readCallback, write_callback_t writeCallback);

  void Power();

  // Private functions
  uint8 read(uint16 address);
  uint16 read16(uint16 address);

  void write(uint16 address, uint8 value);
  void write16(uint16 address, uint16 value);

  uint8 pop();
  uint16 pop16();

  void push(uint8 value);
  void push16(uint16 value);

  void tick();
  void executeInstruction();
};

} // namespace nesturbia

#endif // NESTURBIA_CPU_HPP_INCLUDED
