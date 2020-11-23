#include <cmath> // TODO temporary

#include "nesturbia/apu.hpp"

namespace nesturbia {

namespace {

constexpr std::array<uint8_t, 32> kLengthCounterLookupTable = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};
} // namespace

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
      // Pulse: envelopes
      for (auto &pulse : pulseChannels) {
        if (pulse.envelope.reload) {
          pulse.envelope.reload = false;
          pulse.envelope.divider = pulse.envelope.volume;
          pulse.envelope.count = 0xf;
        } else {
          if (pulse.envelope.divider == 0) {
            pulse.envelope.divider = pulse.envelope.volume;

            if (pulse.envelope.count != 0 || pulse.envelope.loop) {
              pulse.envelope.count = (pulse.envelope.count - 1) & 0xf;
            }
          } else {
            --pulse.envelope.divider;
          }
        }
      }

      // Triangle: linear counter
      if (triangleChannel.linearCounter.reload) {
        triangleChannel.linearCounter.value = triangleChannel.linearCounter.load;
      } else {
        if (triangleChannel.linearCounter.value != 0) {
          --triangleChannel.linearCounter.value;
        }
      }

      if (!triangleChannel.linearCounter.control) {
        // Disable reloading if bit 7 of $4008 is cleared
        triangleChannel.linearCounter.reload = false;
      }

      // Noise: envelope
      if (noiseChannel.envelope.reload) {
        noiseChannel.envelope.reload = false;
        noiseChannel.envelope.divider = noiseChannel.envelope.volume;
        noiseChannel.envelope.count = 0xf;
      } else {
        if (noiseChannel.envelope.divider == 0) {
          noiseChannel.envelope.divider = noiseChannel.envelope.volume;

          if (noiseChannel.envelope.count != 0 || noiseChannel.envelope.loop) {
            noiseChannel.envelope.count = (noiseChannel.envelope.count - 1) & 0xf;
          }
        } else {
          --noiseChannel.envelope.divider;
        }
      }
    }

    // Half frames occur on step 2 and 4
    if (isStep2 || isStep4) {
      // Pulse: sweep + length counters
      bool isPulse1 = true;
      for (auto &pulse : pulseChannels) {
        // Sweep
        if (pulse.sweep.reload) {
          // TODO this logic is duplicated below
          if (pulse.sweep.enabled) {
            pulse.sweep.divider = pulse.sweep.period;
            if (pulse.sweep.enabled && pulse.sweep.shiftAmount != 0) {
              auto periodShifted = pulse.period >> pulse.sweep.shiftAmount;
              if (pulse.sweep.negate) {
                // Get the one's complement (used by pulse 1)
                periodShifted = ~periodShifted;
                if (!isPulse1) {
                  // Pulse 2 uses the two's complement (one's complement + 1)
                  ++periodShifted;
                }
              }

              pulse.period += periodShifted;
            }
          }

          pulse.sweep.reload = false;
          pulse.sweep.divider = pulse.sweep.period;
        } else {
          if (pulse.sweep.divider == 0) {
            // TODO this logic is duplicated above
            if (pulse.sweep.enabled) {
              pulse.sweep.divider = pulse.sweep.period;
              if (pulse.sweep.enabled && pulse.sweep.shiftAmount != 0) {
                auto periodShifted = pulse.period >> pulse.sweep.shiftAmount;
                if (pulse.sweep.negate) {
                  // Get the one's complement (used by pulse 1)
                  periodShifted = ~periodShifted;
                  if (!isPulse1) {
                    // Pulse 2 uses the two's complement (one's complement + 1)
                    ++periodShifted;
                  }
                }

                pulse.period += periodShifted;
              }
            }
          } else {
            --pulse.sweep.divider;
          }
        }

        // Length counter
        if (!pulse.length.halt && pulse.length.value != 0) {
          --pulse.length.value;
        }

        isPulse1 = false;
      }

      // Triangle: length counter
      if (!triangleChannel.length.halt && triangleChannel.length.value != 0) {
        --triangleChannel.length.value;
      }

      // Noise: length counter
      if (!noiseChannel.length.halt && noiseChannel.length.value != 0) {
        --noiseChannel.length.value;
      }

      // TODO other waveforms?
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

    // TODO should this be on even cycles?
    // Pulse channel code that's executed every APU cycle
    for (auto &pulse : pulseChannels) {
      if (pulse.timerCounter == 0) {
        pulse.timerCounter = pulse.period;

        // 3-bit value that wraps around
        pulse.dutyIndex = (pulse.dutyIndex - 1) & 0x7;
      } else {
        --pulse.timerCounter;
      }
    }

    // Noise channel timer
    if (noiseChannel.timerCounter == 0) {
      noiseChannel.timerCounter = noiseChannel.period;

      const auto bit0 = noiseChannel.shiftRegister.bit(0);
      const auto bit1 = noiseChannel.shiftRegister.bit((noiseChannel.envelope.loop ? 6 : 1));
      // if (bit0)
      // throw "";

      noiseChannel.shiftRegister >>= 1;
      noiseChannel.shiftRegister |= (bit0 ^ bit1) << 14;
    } else {
      --noiseChannel.timerCounter;
    }
  }

  // Triangle channel code that's executed every CPU cycle
  // (most APU channel code executes on an APU cycle, which is 2 CPU cycles)
  if (triangleChannel.timerCounter == 0) {
    triangleChannel.timerCounter = triangleChannel.period;

    if (triangleChannel.length.value != 0 && triangleChannel.linearCounter.value != 0) {
      // 5-bit value that wraps around
      triangleChannel.dutyIndex = (triangleChannel.dutyIndex + 1) & 0x1f;
    }
  } else {
    --triangleChannel.timerCounter;
  }

  isOddCycle = !isOddCycle;

  static double sampleSum = 0;
  static uint32_t numSamples = 0;

  // Pulse output
  for (auto &pulse : pulseChannels) {
    if (!pulse.enabled || pulse.length.value == 0) {
      continue;
    }

    // TODO move somewhere else?
    constexpr std::array<uint8_t, 4> kPulseDuty = {0x40, 0x60, 0x78, 0x9f};
    if (!uint8(kPulseDuty[pulse.duty]).bit(pulse.dutyIndex)) {
      continue;
    }

    // TODO double check this
    if (pulse.period < 8 || pulse.timerCounter > 0x7ff) {
      continue;
    }

    // TODO: do proper mixing logic
    sampleSum += 0.00752 * (pulse.envelope.disabled ? pulse.envelope.volume : pulse.envelope.count);
  }

  // Triangle output
  if (triangleChannel.enabled && triangleChannel.length.value != 0 &&
      triangleChannel.linearCounter.value != 0) {
    // TODO move somewhere else?
    constexpr std::array<uint8_t, 32> kTriangleTable = {15, 14, 13, 12, 11, 10, 9,  8,  7,  6, 5,
                                                        4,  3,  2,  1,  0,  0,  1,  2,  3,  4, 5,
                                                        6,  7,  8,  9,  10, 11, 12, 13, 14, 15};

    // TODO: do proper mixing logic
    sampleSum += 0.00851 * kTriangleTable[triangleChannel.dutyIndex];
  }

  // Noise output
  if (noiseChannel.enabled && noiseChannel.length.value != 0 &&
      !noiseChannel.shiftRegister.bit(0)) {
    // TODO: do proper mixing logic
    sampleSum += 0.00494 * (noiseChannel.envelope.disabled ? noiseChannel.envelope.volume
                                                           : noiseChannel.envelope.count);
  }

  ++numSamples;

  static double elapsedCycles = 0.0;
  if (++elapsedCycles > ticksPerSample) {
    // New sample
    elapsedCycles -= ticksPerSample;

    if (sampleCallback) {
      sampleCallback(sampleSum / numSamples / 4);
    }

    sampleSum = 0;
    numSamples = 0;
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
  const auto pulseChannel = static_cast<uint8>(address.bit(2));

  switch (address) {
  case 0x4000:
  case 0x4004:
    pulseChannels[pulseChannel].duty = (value.bit(7) << 1) | value.bit(6);
    pulseChannels[pulseChannel].length.halt = value.bit(5);
    pulseChannels[pulseChannel].envelope.loop = value.bit(5);
    pulseChannels[pulseChannel].envelope.disabled = value.bit(4);
    pulseChannels[pulseChannel].envelope.volume = value & 0xf;
    break;

  case 0x4001:
  case 0x4005:
    pulseChannels[pulseChannel].sweep.enabled = value.bit(7);
    pulseChannels[pulseChannel].sweep.period = ((value >> 4) & 0x7) + 1;
    pulseChannels[pulseChannel].sweep.negate = value.bit(3);
    pulseChannels[pulseChannel].sweep.shiftAmount = value & 0x7;

    // Side effect: set the reload flag
    pulseChannels[pulseChannel].sweep.reload = true;

    // TODO double-check this
    pulseChannels[pulseChannel].envelope.reload = true;
    break;

  case 0x4002:
  case 0x4006:
    pulseChannels[pulseChannel].period &= 0x700;
    pulseChannels[pulseChannel].period |= value;
    break;

  case 0x4003:
  case 0x4007:
    pulseChannels[pulseChannel].period &= 0x0ff;
    pulseChannels[pulseChannel].period |= (value & 0x7) << 8;

    // Length counter load (L)
    pulseChannels[pulseChannel].length.value = kLengthCounterLookupTable[value >> 3];

    // Side effect: Reset the 3-bit ([0-7]) index into the duty cycle table
    pulseChannels[pulseChannel].dutyIndex = 0;

    // Side effect: Reload the pulse channel envelope next APU tick
    pulseChannels[pulseChannel].envelope.reload = true;
    break;

  case 0x4008:
    triangleChannel.length.halt = value.bit(7);

    triangleChannel.linearCounter.control = value.bit(7);
    triangleChannel.linearCounter.load = value & 0x7f;
    break;

  case 0x400a:
    // Timer (T) low
    triangleChannel.period &= 0x700;
    triangleChannel.period |= value;
    break;

  case 0x400b: {
    // Timer (T) high (top 3 bits)
    triangleChannel.period &= 0x0ff;
    triangleChannel.period |= (value & 0x7) << 8;

    triangleChannel.timerCounter = triangleChannel.period;

    // Length counter load (L)
    // TODO move somewhere else?
    constexpr std::array<uint8_t, 32> kTriangleOutput = {15, 14, 13, 12, 11, 10, 9,  8,  7,  6, 5,
                                                         4,  3,  2,  1,  0,  0,  1,  2,  3,  4, 5,
                                                         6,  7,  8,  9,  10, 11, 12, 13, 14, 15};

    triangleChannel.length.value = kTriangleOutput[value >> 3];

    // Reload the linear counter
    triangleChannel.linearCounter.reload = true;

    // Clear the duty cycle index
    triangleChannel.dutyIndex = 0;
  } break;

  case 0x400c:
    noiseChannel.length.halt = value.bit(5);
    noiseChannel.envelope.loop = value.bit(5);
    noiseChannel.envelope.disabled = value.bit(4);
    noiseChannel.envelope.volume = value & 0xf;
    noiseChannel.envelope.count = value & 0xf;

    // TODO double-check this
    noiseChannel.envelope.reload = true;
    break;

  case 0x400e: {
    noiseChannel.envelope.loop = value.bit(7);

    // TODO move somewhere else?
    constexpr std::array<uint16_t, 16> kNoisePeriod = {4,   8,   16,  32,  64,  96,   128,  160,
                                                       202, 254, 380, 508, 762, 1016, 2034, 4068};

    noiseChannel.period = kNoisePeriod[value & 0xf];
  } break;

  case 0x400f:
    // Length counter load (L)
    noiseChannel.length.value = kLengthCounterLookupTable[value >> 3];

    // Side effect: Reload the noise channel envelope next APU tick
    noiseChannel.envelope.reload = true;
    break;

  case 0x4010:
  case 0x4011:
  case 0x4012:
  case 0x4013:
    // TODO
    break;

  case 0x4015:
    // TODO other channel (DMC)
    noiseChannel.enabled = value.bit(3);
    if (!noiseChannel.enabled) {
      noiseChannel.length.value = 0;
    }

    triangleChannel.enabled = value.bit(2);
    if (!triangleChannel.enabled) {
      triangleChannel.length.value = 0;
    }

    pulseChannels[1].enabled = value.bit(1);
    if (!pulseChannels[1].enabled) {
      pulseChannels[1].length.value = 0;
    }

    pulseChannels[0].enabled = value.bit(0);
    if (!pulseChannels[0].enabled) {
      pulseChannels[0].length.value = 0;
    }
    break;

  case 0x4017:
    frameCounter.mode = value.bit(7);
    frameCounter.interruptInhibit = value.bit(6);

    // TODO: NESDEV says that timer is reset 3 to 4 cycles after this register is written
    frameCounter.shiftRegister = 0x7fff;

    // TODO: NESDEV says that quarter/half frame signals generated if mode flag is set
    break;
  }
}

} // namespace nesturbia
