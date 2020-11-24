#ifndef NESTURBIA_MAPPER_HPP_INCLUDED
#define NESTURBIA_MAPPER_HPP_INCLUDED

#include <memory>
#include <string>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Mapper {
  // Types
  using ptr_t = std::unique_ptr<Mapper>;

  // TODO: needs a 4-way type?
  enum class mirror_t { horizontal, vertical, oneScreenLower, oneScreenHigher };

  // Public functions
  [[nodiscard]] virtual mirror_t GetMirrorType() const = 0;

  virtual uint8 ReadPRG(uint16 address) = 0;
  virtual void WritePRG(uint16 address, uint8 value) = 0;

  virtual uint8 ReadCHR(uint16 address) = 0;
  virtual void WriteCHR(uint16 address, uint8 value) = 0;
};

} // namespace nesturbia

#endif // NESTURBIA_MAPPER_HPP_INCLUDED
