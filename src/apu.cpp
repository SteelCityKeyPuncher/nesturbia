#include <cmath> // TODO temporary

#include "nesturbia/apu.hpp"

namespace nesturbia {

void Apu::SetSampleCallback(sample_callback_t sampleCallback, uint32_t sampleRate) {
  this->sampleCallback = sampleCallback;
  ticksPerSample = 89341.5 / 3.0 * 60.0 / sampleRate;
}

void Apu::Tick() {
  static double phase;
  static double sampleSum = 0;
  static uint32_t numSamples = 0;

  sampleSum += sin(phase) / 8.0;
  phase += 2 * 3.14159265358979323846 * 440.0 / 1789773.0;
  ++numSamples;

  static double elapsedCycles = 0.0;
  if (++elapsedCycles > ticksPerSample) {
    // New sample
    elapsedCycles -= ticksPerSample;

    const float sample = sampleSum / numSamples;
    sampleSum = 0;
    numSamples = 0;

    if (sampleCallback) {
      sampleCallback(sample);
    }
  }
}

} // namespace nesturbia
