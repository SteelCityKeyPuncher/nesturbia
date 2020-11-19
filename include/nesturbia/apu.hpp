#ifndef NESTURBIA_APU_HPP_INCLUDED
#define NESTURBIA_APU_HPP_INCLUDED

#include <array>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Apu {
  // Types
  using sample_callback_t = void (*)(float);

  struct frame_counter_t {
    bool resetShiftRegister;
    uint16 shiftRegister;
    bool interruptInhibit;
    bool mode;
  };

  // Data
  sample_callback_t sampleCallback = nullptr;
  double ticksPerSample = 0;

  frame_counter_t frameCounter;

  bool isOddCycle;

  // Public functions
  void Power();
  void SetSampleCallback(sample_callback_t sampleCallback, uint32_t sampleRate);
  void Tick();
  uint8 ReadRegister(uint16 address);
  void WriteRegister(uint16 address, uint8 value);
};

} // namespace nesturbia

#endif // NESTURBIA_APU_HPP_INCLUDED
