#include <cstdlib>
#include <iostream>

#include "nesturbia/nesturbia.hpp"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Expected one argument (ROM path)." << std::endl;
    return EXIT_FAILURE;
  }

  nesturbia::Nesturbia emulator;

  if (!emulator.LoadRom(argv[1])) {
    std::cerr << "Could not load ROM '" << argv[1] << "'" << std::endl;
    return EXIT_FAILURE;
  }

  emulator.Run();

  return EXIT_SUCCESS;
}
