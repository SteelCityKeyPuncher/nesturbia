#ifndef NESTURBIA_MAPPERS_MAPPER_0_HPP_INCLUDED
#define NESTURBIA_MAPPERS_MAPPER_0_HPP_INCLUDED

#include <vector>

#include "nesturbia/mapper.hpp"

namespace nesturbia {

struct Mapper0 : public Mapper {
  // Data
  std::vector<uint8> prgRom;
  std::vector<uint8> chrRom;
  mirror_t mirrorType;

  // Public functions
  static Mapper::ptr_t Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom,
                              mirror_t mirrorType);

  [[nodiscard]] mirror_t GetMirrorType() const override;

  uint8 Read(uint16 address) override;
  void Write(uint16 address, uint8 value) override;

  uint8 ReadChr(uint16 address) override;
  void WriteChr(uint16 address, uint8 value) override;
};

} // namespace nesturbia

#endif // NESTURBIA_MAPPERS_MAPPER_0_HPP_INCLUDED
