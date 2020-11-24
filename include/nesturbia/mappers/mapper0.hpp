#ifndef NESTURBIA_MAPPERS_MAPPER_0_HPP_INCLUDED
#define NESTURBIA_MAPPERS_MAPPER_0_HPP_INCLUDED

#include <vector>

#include "nesturbia/mapper.hpp"

namespace nesturbia {

// Mapper 0: aka NROM
struct Mapper0 : public Mapper {
  // Data
  std::vector<uint8> prgRom;
  std::vector<uint8> chrRom;
  mirror_t mirrorType;

  // Public functions
  static Mapper::ptr_t Create(const std::vector<uint8> &prgRom, const std::vector<uint8> &chrRom,
                              mirror_t mirrorType);

  [[nodiscard]] mirror_t GetMirrorType() const override;

  uint8 ReadPRG(uint16 address) override;
  void WritePRG(uint16 address, uint8 value) override;

  uint8 ReadCHR(uint16 address) override;
  void WriteCHR(uint16 address, uint8 value) override;
};

} // namespace nesturbia

#endif // NESTURBIA_MAPPERS_MAPPER_0_HPP_INCLUDED
