# nesturbia

A NES emulator written in C++17.

## Features

* [ ] CPU
  * [x] Legal opcodes
  * [ ] Illegal opcodes
  * [ ] IRQ support
* [ ] PPU
  * [x] Complete background and sprite rendering
  * [ ] Support for different palettes
* [ ] APU
  * [x] Pulse (square wave) channel support
  * [x] Triangle channel support
  * [ ] Noise channel support
  * [ ] DMC channel support
* [ ] Cartridges
  * [x] Mapper 0 (NROM) support
  * [ ] Mapper 1 (SxROM) support - mostly implemented
  * [ ] Expansion ROM support
  * [ ] Other mappers
  * [ ] Battery-backed memory
* [ ] Player input
  * [x] Joypad/controller 1 support
  * [ ] Joypad/controller 2 support
  * [ ] Zapper support
  * [ ] Ability to configure/map inputs

## How to Compile

### Dependencies

Install the following dependencies:
* git
* CMake

### Getting the Source Code

```bash
git clone https://github.com/SteelCityKeyPuncher/nesturbia.git
cd nesturbia
git submodule init
git submodule update
```

### Compiling

Note: compilation has only been tested on Linux so far.

```bash
cmake -S . -B build
cmake --build build
```
