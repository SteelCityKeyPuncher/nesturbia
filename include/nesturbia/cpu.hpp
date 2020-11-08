#ifndef NESTURBIA_CPU_HPP_INCLUDED
#define NESTURBIA_CPU_HPP_INCLUDED

#include <cstdint>
#include <functional>

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

    auto &operator=(uint8_t value) {
      C = ((value & (1U << 0)) != 0);
      Z = ((value & (1U << 1)) != 0);
      I = ((value & (1U << 2)) != 0);
      D = ((value & (1U << 3)) != 0);
      V = ((value & (1U << 6)) != 0);
      N = ((value & (1U << 7)) != 0);

      return *this;
    }

    operator unsigned() const { return C << 0 | Z << 1 | I << 2 | D << 3 | V << 6 | N << 7; }
  };

  using read_callback_t = std::function<uint8_t(uint16_t)>;
  using write_callback_t = std::function<void(uint16_t, uint8_t)>;

  // Data
  uint8_t A;
  uint8_t X;
  uint8_t Y;
  uint8_t S;
  uint16_t PC;
  flags_t P;

  read_callback_t readCallback;
  write_callback_t writeCallback;

  uint32_t cycles;

  // Public functions
  Cpu(read_callback_t readCallback, write_callback_t writeCallback);

  void Power();

  // Private functions
  uint8_t read(uint16_t address);
  uint16_t read16(uint16_t address);

  void write(uint16_t address, uint8_t value);
  void write16(uint16_t address, uint16_t value);
};
} // namespace nesturbia

#endif // NESTURBIA_CPU_HPP_INCLUDED
