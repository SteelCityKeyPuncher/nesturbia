#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "nesturbia/nesturbia.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Expected one argument (ROM path)." << std::endl;
    return EXIT_FAILURE;
  }

  auto file = std::ifstream(argv[1], std::ios::binary);
  if (!file) {
    std::cerr << "Could not open ROM '" << argv[1] << "'" << std::endl;
    return EXIT_FAILURE;
  }

  std::vector<uint8_t> rom;
  rom.assign(std::istreambuf_iterator<char>(file), {});

  nesturbia::Nesturbia emulator;

  if (!emulator.LoadRom(rom.data(), rom.size())) {
    std::cerr << "Could not load ROM '" << argv[1] << "'" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
