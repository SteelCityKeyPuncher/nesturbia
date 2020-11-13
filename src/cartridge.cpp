#include "nesturbia/cartridge.hpp"

namespace nesturbia {

bool Cartridge::LoadRom(const void *romData, size_t romDataSize) {
  mapper = Mapper::Create(romData, romDataSize);
  if (!mapper) {
    return false;
  }

  return true;
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
