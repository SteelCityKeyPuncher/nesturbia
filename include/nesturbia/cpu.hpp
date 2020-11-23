#ifndef NESTURBIA_CPU_HPP_INCLUDED
#define NESTURBIA_CPU_HPP_INCLUDED

#include <cstdint>
#include <functional>

#include "nesturbia/types.hpp"

namespace nesturbia {

struct Cpu {
  // Types
  struct flags_t {
    bool C = false;
    bool Z = false;
    bool I = false;
    bool D = false;
    bool V = false;
    bool N = false;

    auto &operator=(uint8 value) {
      C = value.bit(0);
      Z = value.bit(1);
      I = value.bit(2);
      D = value.bit(3);
      V = value.bit(6);
      N = value.bit(7);

      return *this;
    }

    operator unsigned() const { return C << 0 | Z << 1 | I << 2 | D << 3 | V << 6 | N << 7; }
  };

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

    length_counter_t length;
    envelope_t envelope;

    // 12-bit value
    uint16 period = 0;

    // 12-bit value
    // TODO where to initialize (other than here obviously)?
    uint16 timerCounter = 0;

    uint16 shiftRegister = 1;
  };

  struct dmc_channel_t {
    bool enabled = false;
    bool irqEnabled = false;
    bool loop = false;

    uint16 period = 0;
    uint16 tickValue = 0;

    // 7-bit value
    uint8 value = 0;

    uint16 address = 0;
    uint16 length = 0;

    uint16 sampleAddress = 0;
    uint16 sampleLength = 0;

    uint8 shiftRegister = 0;
    uint8 bitCount = 0;
  };

  struct frame_counter_t {
    bool resetShiftRegister;
    uint16 shiftRegister;
    bool interruptInhibit;
    bool mode;
  };

  using read_callback_t = std::function<uint8(uint16)>;
  using write_callback_t = std::function<void(uint16, uint8)>;
  using tick_callback_t = std::function<void(void)>;
  using sample_callback_t = void (*)(float);

  // Data
  uint8 A;
  uint8 X;
  uint8 Y;
  uint8 S;
  uint16 PC;
  flags_t P;

  read_callback_t readCallback;
  write_callback_t writeCallback;
  tick_callback_t tickCallback;
  sample_callback_t sampleCallback = nullptr;

  uint32_t cycles;

  bool nmi;

  // APU-specific
  double ticksPerSample = 0;

  std::array<pulse_channel_t, 2> pulseChannels;
  triangle_channel_t triangleChannel;
  noise_channel_t noiseChannel;
  dmc_channel_t dmcChannel;
  frame_counter_t frameCounter;

  bool isOddCycle;

  // Public functions
  Cpu(read_callback_t readCallback, write_callback_t writeCallback, tick_callback_t tickCallback);

  void Power();
  void NMI();

  void SetSampleCallback(sample_callback_t sampleCallback, uint32_t sampleRate);

  // Private functions
  uint8 read(uint16 address);
  uint16 read16(uint16 address);

  void write(uint16 address, uint8 value);
  void write16(uint16 address, uint16 value);

  uint8 pop();
  uint16 pop16();

  void push(uint8 value);
  void push16(uint16 value);

  void tick();
  void executeInstruction();
};

} // namespace nesturbia

#endif // NESTURBIA_CPU_HPP_INCLUDED
