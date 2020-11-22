// TODO: temporary for MD5 below
#include <iostream>

#include "nesturbia/cartridge.hpp"
#include "nesturbia/util/md5.hpp"

namespace nesturbia {

bool Cartridge::LoadRom(const void *romData, size_t romDataSize) {
  mapper = Mapper::Create(romData, romDataSize);
  if (!mapper) {
    return false;
  }

  // TODO: temporary testing
  std::cout << "MD5: " << md5(reinterpret_cast<const uint8 *>(romData) + 16, romDataSize - 16)
            << std::endl;

  return true;
}

Mapper::mirror_t Cartridge::GetMirrorType() const {
  if (mapper) {
    return mapper->GetMirrorType();
  }

  return Mapper::mirror_t::horizontal;
}

uint8 Cartridge::Read(uint16 address) {
  if (mapper) {
    return mapper->Read(address);
  }

  return 0;
}

void Cartridge::Write(uint16 address, uint8 value) {
  if (mapper) {
    mapper->Write(address, value);
  }
}

uint8 Cartridge::ReadChr(uint16 address) {
  if (mapper) {
    return mapper->ReadChr(address);
  }

  return 0;
}

void Cartridge::WriteChr(uint16 address, uint8 value) {
  if (mapper) {
    mapper->WriteChr(address, value);
  }
}

} // namespace nesturbia
