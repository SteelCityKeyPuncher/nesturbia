#ifndef NESTURBIA_APU_HPP_INCLUDED
#define NESTURBIA_APU_HPP_INCLUDED

#include <array>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Apu {
  // Types
  using sample_callback_t = void (*)(float);

  struct length_counter_t {
    // TODO halt isn't cleared on reset for triangle (keep in mind if implementing reset behavior)
    bool halt = false;
    // 5-bit value
    uint8 value = 0;
  };

  struct envelope_t {
    bool loop = false;
    // Note: called 'constant volume' in some places
    bool disabled = false;
    // 4-bit value
    uint8 volume = 0;
    // 4-bit value
    uint8 divider = 0;
    // 4-bit value
    uint8 count = 0;
    bool reload = false;
  };

  struct pulse_channel_t {
    bool enabled = false;

    // 2-bit value
    uint8 duty = 0;
    // 3-bit value, wraps around
    uint8 dutyIndex = 0;

    length_counter_t length;
    envelope_t envelope;

    struct {
      bool enabled = false;
      // 3-bit value
      uint8 period = 0;
      // 3-bit value
      uint8 divider = 0;
      bool negate = false;
      uint8 shiftAmount = 0;
      bool reload = false;
    } sweep;

    // 11-bit value
    uint16 period;

    // 11-bit value
    // TODO where to initialize (other than here obviously)?
    uint16 timerCounter = 0;
  };

  struct triangle_channel_t {
    bool enabled = false;

    length_counter_t length;

    // 5-bit value, wraps around
    uint8 dutyIndex = 0;

    struct {
      bool control;
      // 7-bit value
      uint8 load = 0;
      // 7-bit value
      uint8 value = 0;
      bool reload = false;
    } linearCounter;

    // 11-bit value
    uint16 period;

    // 11-bit value
    // TODO where to initialize (other than here obviously)?
    uint16 timerCounter = 0;
  };

  struct noise_channel_t {
    bool enabled = false;
    bool loop = false;

    length_counter_t length;
    envelope_t envelope;

    // 12-bit value
    uint16 period = 0;

    // 12-bit value
    // TODO where to initialize (other than here obviously)?
    uint16 timerCounter = 0;

    uint16 shiftRegister = 0;
  };

  struct frame_counter_t {
    bool resetShiftRegister;
    uint16 shiftRegister;
    bool interruptInhibit;
    bool mode;
  };

  // Data
  sample_callback_t sampleCallback = nullptr;
  double ticksPerSample = 0;

  std::array<pulse_channel_t, 2> pulseChannels;
  triangle_channel_t triangleChannel;
  noise_channel_t noiseChannel;
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
