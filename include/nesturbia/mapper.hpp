#ifndef NESTURBIA_MAPPER_HPP_INCLUDED
#define NESTURBIA_MAPPER_HPP_INCLUDED

#include <memory>
#include <string>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Mapper {
  // Types
  using ptr_t = std::unique_ptr<Mapper>;
  enum class mirror_t { horizontal = 0, vertical = 1 };

  // Public functions
  static ptr_t Create(const void *romData, size_t romDataSize);

  [[nodiscard]] virtual mirror_t GetMirrorType() const = 0;

  virtual uint8 Read(uint16 address) = 0;
  virtual void Write(uint16 address, uint8 value) = 0;

  virtual uint8 ReadChr(uint16 address) = 0;
  virtual void WriteChr(uint16 address, uint8 value) = 0;
};

} // namespace nesturbia

#endif // NESTURBIA_MAPPER_HPP_INCLUDED
