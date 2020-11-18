#ifndef NESTURBIA_APU_HPP_INCLUDED
#define NESTURBIA_APU_HPP_INCLUDED

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Apu {
  // Types
  using sample_callback_t = void (*)(float);

  // Data
  sample_callback_t sampleCallback = nullptr;
  double ticksPerSample = 0;

  // Public functions
  void SetSampleCallback(sample_callback_t sampleCallback, uint32_t sampleRate);
  void Tick();
};

} // namespace nesturbia

#endif // NESTURBIA_APU_HPP_INCLUDED
