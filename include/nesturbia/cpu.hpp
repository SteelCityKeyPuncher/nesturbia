#ifndef NESTURBIA_CPU_HPP_INCLUDED
#define NESTURBIA_CPU_HPP_INCLUDED

#include <cstdint>
#include <functional>

namespace nesturbia {
struct cpu {
  // Types
  using read_callback_t = std::function<uint8_t(uint16_t)>;
  using write_callback_t = std::function<void(uint16_t, uint8_t)>;

  // Data
  read_callback_t readCallback;
  write_callback_t writeCallback;

  // Public functions
  cpu(read_callback_t readCallback, write_callback_t writeCallback);

  // Private functions
  uint8_t read(uint16_t address);
  uint16_t read16(uint16_t address);

  void write(uint16_t address, uint8_t value);
  void write16(uint16_t address, uint16_t value);
};
} // namespace nesturbia

#endif // NESTURBIA_CPU_HPP_INCLUDED
