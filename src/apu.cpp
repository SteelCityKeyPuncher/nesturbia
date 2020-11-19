#include <cmath> // TODO temporary

#include "nesturbia/apu.hpp"

namespace nesturbia {

void Apu::Power() {
  frameCounter.resetShiftRegister = false;
  frameCounter.shiftRegister = 0x7fff;
  frameCounter.interruptInhibit = false;
  frameCounter.mode = false;

  isOddCycle = false;
}

void Apu::SetSampleCallback(sample_callback_t sampleCallback, uint32_t sampleRate) {
  this->sampleCallback = sampleCallback;
  ticksPerSample = 89341.5 / 3.0 * 60.0 / sampleRate;
}

void Apu::Tick() {
  if (isOddCycle) {
    if (frameCounter.resetShiftRegister) {
      frameCounter.resetShiftRegister = false;
      frameCounter.shiftRegister = 0x7fff;
    } else {
      // 14-bit linear feedback shift register
      // The top two bits are XORed and put into bit 0
      // The rest of the bits are shifted by one
      frameCounter.shiftRegister =
          ((frameCounter.shiftRegister << 1) & 0x7ffe) |
          (frameCounter.shiftRegister.bit(14) ^ frameCounter.shiftRegister.bit(13));
    }

    const auto isStep1 = frameCounter.shiftRegister == 0x1061;
    const auto isStep2 = frameCounter.shiftRegister == 0x3603;
    const auto isStep3 = frameCounter.shiftRegister == 0x2cd3;
    const auto isStep4 = (!frameCounter.mode && frameCounter.shiftRegister == 0x0a1f) ||
                         frameCounter.shiftRegister == 0x7185;

    // Quarter frames occur every step
    if (isStep1 || isStep2 || isStep3 || isStep4) {
      //
    }

    // Half frames occur on step 2 and 4
    if (isStep2 || isStep4) {
      //
    }

    // Set the IRQ on step 4 (mode 0 only)
    // TODO the IRQ is set continuously in this condition, not just in this tick
    if (isStep4 && !frameCounter.mode) {
      //
    }

    // Flag that the shift register should reset from step 4
    if (isStep4) {
      frameCounter.resetShiftRegister = true;
    }
  }

  isOddCycle = !isOddCycle;

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

uint8 Apu::ReadRegister(uint16 address) {
  if (address == 0x4015) {
    // TODO
    return 0;
  }

  return 0;
}

void Apu::WriteRegister(uint16 address, uint8 value) {
  // Used for pulse configuration registers ($4000-4007) only
  // TODO const auto pulseChannel = static_cast<uint8>(address.bit(3));

  switch (address) {
  case 0x4000:
  case 0x4004:
    // TODO
    break;

  case 0x4001:
  case 0x4005:
    // TODO
    break;

  case 0x4002:
  case 0x4006:
    // TODO
    break;

  case 0x4003:
  case 0x4007:
    // TODO
    break;

  case 0x4008:
  case 0x400a:
  case 0x400b:
  case 0x400c:
  case 0x400e:
  case 0x400f:
  case 0x4010:
  case 0x4011:
  case 0x4012:
  case 0x4013:
  case 0x4015:
    // TODO
    break;

  case 0x4017:
    frameCounter.mode = value.bit(7);
    frameCounter.interruptInhibit = value.bit(6);

    // TODO: NESDEV says that timer is reset 3 to 4 cycles after this register is written
    frameCounter.shiftRegister = 0x7fff;

    // TODO: NESDEV says that quarter/half frame signals generated if mode flag is set
    break;

  default:
    break;
  }
}

} // namespace nesturbia
