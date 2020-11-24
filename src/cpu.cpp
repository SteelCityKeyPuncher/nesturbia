#include <array>
#include <utility>

#include "nesturbia/cpu.hpp"

namespace nesturbia {

namespace {

extern const std::array<void (*)(Cpu &), 256> instructions;

constexpr std::array<uint8_t, 32> kLengthCounterLookupTable = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30};

} // namespace

Cpu::Cpu(read_callback_t readCallback, write_callback_t writeCallback, tick_callback_t tickCallback)
    : readCallback(std::move(readCallback)), writeCallback(std::move(writeCallback)),
      tickCallback(std::move(tickCallback)) {}

void Cpu::Power() {
  A = 0x00;
  X = 0x00;
  Y = 0x00;
  S = 0xfd;
  PC = read16(0xfffc);
  P = 0x4;

  cycles = 7;

  nmi = false;

  // APU-specific
  frameCounter.resetShiftRegister = false;
  frameCounter.shiftRegister = 0x7fff;
  frameCounter.interruptInhibit = false;
  frameCounter.mode = false;

  // TODO: should the APU run while the CPU resets?
  // This is technically 'even' if 7 cycles ran during a reset
  isOddCycle = false;
}

void Cpu::NMI() { nmi = true; }

void Cpu::SetSampleCallback(sample_callback_t sampleCallback, uint32_t sampleRate) {
  this->sampleCallback = sampleCallback;

  // TODO: document where these numbers came from
  ticksPerSample = 89341.5 / 3.0 * 60.0 / sampleRate;
}

uint8 Cpu::read(uint16 address) {
  tick();

  if (address >= 0x4000 && address < 0x4015) {
    return 0;
  }

  if (address == 0x4015) {
    // TODO: APU status register
    return 0;
  }

  if (address > 0x4017 && address < 0x4020) {
    return 0;
  }

  if (!readCallback) {
    return 0xff;
  }

  return readCallback(address);
}

uint16 Cpu::read16(uint16 address) {
  return read(address) | (read(static_cast<uint16>(address + 1)) << 8);
}

void Cpu::write(uint16 address, uint8 value) {
  tick();

  switch (address) {
  case 0x4000:
  case 0x4004:
    pulseChannels[static_cast<uint8>(address.bit(2))].duty = (value.bit(7) << 1) | value.bit(6);
    pulseChannels[static_cast<uint8>(address.bit(2))].length.halt = value.bit(5);
    pulseChannels[static_cast<uint8>(address.bit(2))].envelope.loop = value.bit(5);
    pulseChannels[static_cast<uint8>(address.bit(2))].envelope.disabled = value.bit(4);
    pulseChannels[static_cast<uint8>(address.bit(2))].envelope.volume = value & 0xf;
    return;

  case 0x4001:
  case 0x4005:
    pulseChannels[static_cast<uint8>(address.bit(2))].sweep.enabled = value.bit(7);
    pulseChannels[static_cast<uint8>(address.bit(2))].sweep.period = ((value >> 4) & 0x7) + 1;
    pulseChannels[static_cast<uint8>(address.bit(2))].sweep.negate = value.bit(3);
    pulseChannels[static_cast<uint8>(address.bit(2))].sweep.shiftAmount = value & 0x7;

    // Side effect: set the reload flag
    pulseChannels[static_cast<uint8>(address.bit(2))].sweep.reload = true;

    // TODO double-check this
    pulseChannels[static_cast<uint8>(address.bit(2))].envelope.reload = true;
    return;

  case 0x4002:
  case 0x4006:
    pulseChannels[static_cast<uint8>(address.bit(2))].period &= 0x700;
    pulseChannels[static_cast<uint8>(address.bit(2))].period |= value;
    return;

  case 0x4003:
  case 0x4007:
    pulseChannels[static_cast<uint8>(address.bit(2))].period &= 0x0ff;
    pulseChannels[static_cast<uint8>(address.bit(2))].period |= (value & 0x7) << 8;

    // Length counter load (L)
    pulseChannels[static_cast<uint8>(address.bit(2))].length.value =
        kLengthCounterLookupTable[value >> 3];

    // Side effect: Reset the 3-bit ([0-7]) index into the duty cycle table
    pulseChannels[static_cast<uint8>(address.bit(2))].dutyIndex = 0;

    // Side effect: Reload the pulse channel envelope next APU tick
    pulseChannels[static_cast<uint8>(address.bit(2))].envelope.reload = true;
    return;

  case 0x4008:
    triangleChannel.length.halt = value.bit(7);

    triangleChannel.linearCounter.control = value.bit(7);
    triangleChannel.linearCounter.load = value & 0x7f;
    return;

  case 0x400a:
    // Timer (T) low
    triangleChannel.period &= 0x700;
    triangleChannel.period |= value;
    return;

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
  }
    return;

  case 0x400c:
    noiseChannel.length.halt = value.bit(5);
    noiseChannel.envelope.loop = value.bit(5);
    noiseChannel.envelope.disabled = value.bit(4);
    noiseChannel.envelope.volume = value & 0xf;
    noiseChannel.envelope.count = value & 0xf;

    // TODO double-check this
    noiseChannel.envelope.reload = true;
    return;

  case 0x400e: {
    noiseChannel.envelope.loop = value.bit(7);

    // TODO move somewhere else?
    constexpr std::array<uint16_t, 16> kNoisePeriod = {4,   8,   16,  32,  64,  96,   128,  160,
                                                       202, 254, 380, 508, 762, 1016, 2034, 4068};

    noiseChannel.period = kNoisePeriod[value & 0xf];
  }
    return;

  case 0x400f:
    // Length counter load (L)
    noiseChannel.length.value = kLengthCounterLookupTable[value >> 3];

    // Side effect: Reload the noise channel envelope next APU tick
    noiseChannel.envelope.reload = true;
    return;

  case 0x4010: {
    dmcChannel.irqEnabled = value.bit(7);
    dmcChannel.loop = value.bit(6);

    // TODO move somewhere else?
    constexpr std::array<uint16_t, 16> kDmcPeriod = {214, 190, 170, 160, 143, 127, 113, 107,
                                                     95,  80,  71,  64,  53,  42,  36,  27};

    dmcChannel.period = kDmcPeriod[value & 0xf];
  }
    return;

  case 0x4011:
    dmcChannel.value = value & 0x7f;
    return;

  case 0x4012:
    dmcChannel.sampleAddress = 0xc000 | (value << 6);
    return;

  case 0x4013:
    dmcChannel.sampleLength = (value << 4) | 0x1;
    return;

  case 0x4015:
    dmcChannel.enabled = value.bit(4);
    if (!dmcChannel.enabled) {
      dmcChannel.length = 0;
    } else if (dmcChannel.length == 0) {
      dmcChannel.address = dmcChannel.sampleAddress;
      dmcChannel.length = dmcChannel.sampleLength;
    }

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
    return;

  case 0x4017:
    frameCounter.mode = value.bit(7);
    frameCounter.interruptInhibit = value.bit(6);

    // TODO: NESDEV says that timer is reset 3 to 4 cycles after this register is written
    frameCounter.shiftRegister = 0x7fff;

    // TODO: NESDEV says that quarter/half frame signals generated if mode flag is set
    return;
  }

  if (writeCallback) {
    writeCallback(address, value);
  }
}

void Cpu::write16(uint16 address, uint16 value) {
  writeCallback(address, static_cast<uint8>(value));
  writeCallback((uint16)(address + 1), static_cast<uint8>(value >> 8));
}

uint8 Cpu::pop() { return read(0x100 + ++S); }

uint16 Cpu::pop16() { return pop() | (static_cast<uint16>(pop()) << 8); }

void Cpu::push(uint8 value) { write(0x100 + S--, value); }

void Cpu::push16(uint16 value) {
  push(value >> 8);
  push(value);
}

void Cpu::tick() {
  ++cycles;
  if (tickCallback) {
    tickCallback();
  }

  // TODO: move into its own function?
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

      noiseChannel.shiftRegister >>= 1;
      noiseChannel.shiftRegister |= (bit0 ^ bit1) << 14;
    } else {
      --noiseChannel.timerCounter;
    }

    // DMC channel timer
    if (dmcChannel.enabled) {
      if (dmcChannel.length != 0 && dmcChannel.bitCount == 0) {
        // TODO: CPU stall?
        // BQS TODO add a 'peek' to prevent infinite recursion
        dmcChannel.shiftRegister = readCallback(dmcChannel.address++);
        dmcChannel.bitCount = 8;

        if (dmcChannel.address == 0) {
          dmcChannel.address = 0x8000;
        }

        --dmcChannel.length;
        if (dmcChannel.length == 0 && dmcChannel.loop) {
          dmcChannel.address = dmcChannel.sampleAddress;
          dmcChannel.length = dmcChannel.sampleLength;
        }
      }

      if (dmcChannel.tickValue == 0) {
        dmcChannel.tickValue = dmcChannel.period;

        if (dmcChannel.bitCount != 0) {
          if (dmcChannel.shiftRegister.bit(0)) {
            if (dmcChannel.value <= 125) {
              dmcChannel.value += 2;
            }
          } else {
            if (dmcChannel.value >= 2) {
              dmcChannel.value -= 2;
            }
          }

          dmcChannel.shiftRegister >>= 1;
          --dmcChannel.bitCount;
        }
      } else {
        --dmcChannel.tickValue;
      }
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

  // DMC output
  // TODO: do proper mixing logic
  sampleSum += 0.00335 * dmcChannel.value;

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

void Cpu::executeInstruction() {
  if (nmi) {
    nmi = false;
    push16(PC);
    push(P | 0x20);
    PC = read16(0xfffa);
    P.I = true;
    tick();
    tick();
    return;
  }

  // TODO IRQ

  const auto opcode = read(PC++);
  instructions[opcode](*this);
}

namespace {

// Helper functions
inline bool checkPageCross(uint16 value, int8_t offset) {
  return (((value + offset) & 0xff00) != (value & 0xff00));
}

inline void branch(Cpu &cpu, bool takeBranch) {
  const auto offset = static_cast<int8_t>(cpu.read(cpu.PC++));
  if (takeBranch) {
    if (checkPageCross(cpu.PC, offset)) {
      cpu.tick();
    }

    cpu.tick();
    cpu.PC += offset;
  }
}

// Addressing modes
using addr_func_t = uint16 (*)(Cpu &);

inline uint16 addr_abs(Cpu &cpu) { return cpu.read16((cpu.PC += 2) - 2); }

template <bool CheckPageCross = true> inline uint16 addr_abx(Cpu &cpu) {
  const auto v = addr_abs(cpu);

  if constexpr (CheckPageCross) {
    if (checkPageCross(v, cpu.X)) {
      // A dummy read occurs here because (address + X) crosses a page boundary
      // * The CPU adds (absolute address) and (register X) without an 8-bit carry
      // * The CPU starts to fetch from the result of (address + X)
      // * The CPU sees that it needs to factor in the carry into the upper 8 bits, and takes
      //   another cycle to read the intended value
      cpu.read((v & 0xff00) | ((v + cpu.X) & 0xff));
    }
  } else {
    cpu.tick();
  }

  return v + cpu.X;
}

inline uint16 addr_aby(Cpu &cpu) {
  const auto v = addr_abs(cpu);

  if (checkPageCross(v, cpu.Y)) {
    cpu.tick();
  }

  return v + cpu.Y;
}

inline uint16 addr_acc(Cpu &) { return 0; }

inline uint16 addr_imm(Cpu &cpu) { return cpu.PC++; }

inline uint16 addr_ind(Cpu &cpu) {
  const auto a = addr_abs(cpu);
  const auto l = cpu.read(a);
  const auto h = cpu.read((a & 0xff00) | ((a + 1) & 0x00ff));
  return l | (h << 8);
}

inline uint16 addr_inx(Cpu &cpu) {
  const auto l = static_cast<uint8>(cpu.read(cpu.PC++) + cpu.X);
  const auto h = static_cast<uint8>(l + 1);
  cpu.tick();
  return cpu.read(l) | (cpu.read(h) << 8);
}

inline uint16 addr_iny(Cpu &cpu) {
  const auto l = cpu.read(cpu.PC++);
  const auto h = static_cast<uint8>(l + 1);
  const auto v = static_cast<uint16>(cpu.read(l) | cpu.read(h) << 8);

  if (checkPageCross(v, cpu.Y)) {
    // A dummy read occurs here because (zero-page + Y) crosses a page boundary
    // * The CPU adds (zero-page) and (register Y) without an 8-bit carry
    // * The CPU starts to fetch from the result of (zero-page + Y)
    // * The CPU sees that it needs to factor in the carry into the upper 8 bits, and takes
    //   another cycle to read the intended value
    cpu.read((v & 0xff00) | ((v + cpu.Y) & 0xff));
  }

  return v + cpu.Y;
}

inline uint16 addr_zpg(Cpu &cpu) { return cpu.read(cpu.PC++); }

inline uint16 addr_zpx(Cpu &cpu) {
  cpu.tick();
  return (cpu.read(cpu.PC++) + cpu.X) & 0xff;
}

inline uint16 addr_zpy(Cpu &cpu) {
  cpu.tick();
  return (cpu.read(cpu.PC++) + cpu.Y) & 0xff;
}

// Instructions/opcodes
using instr_func_t = void (*)(Cpu &);

template <addr_func_t T> static void op_adc(Cpu &cpu) {
  const auto v = cpu.read(T(cpu));
  const uint16 r16 = cpu.A + v + cpu.P.C;
  const auto r = static_cast<uint8>(r16);

  cpu.P.C = r16.bit(8);
  cpu.P.Z = r == 0;
  cpu.P.V = (~(cpu.A ^ v) & (cpu.A ^ r) & 0x80) != 0;
  cpu.P.N = r.bit(7);

  cpu.A = r;
}

template <addr_func_t T> static void op_and(Cpu &cpu) {
  cpu.A &= cpu.read(T(cpu));

  cpu.P.Z = cpu.A == 0;
  cpu.P.N = cpu.A.bit(7);
}

template <addr_func_t T> static void op_asl(Cpu &cpu) {
  cpu.tick();

  if (T == addr_acc) {
    const auto r = static_cast<uint8>(cpu.A << 1);

    cpu.P.C = cpu.A.bit(7);
    cpu.P.Z = r == 0;
    cpu.P.N = r.bit(7);

    cpu.A = r;
  } else {
    const auto address = T(cpu);
    const auto v = cpu.read(address);
    const auto r = static_cast<uint8>(v << 1);

    cpu.P.C = v.bit(7);
    cpu.P.Z = r == 0;
    cpu.P.N = r.bit(7);

    cpu.write(address, r);
  }
}

static void op_bcc(Cpu &cpu) { branch(cpu, !cpu.P.C); }

static void op_bcs(Cpu &cpu) { branch(cpu, cpu.P.C); }

static void op_beq(Cpu &cpu) { branch(cpu, cpu.P.Z); }

template <addr_func_t T> static void op_bit(Cpu &cpu) {
  const auto v = cpu.read(T(cpu));

  cpu.P.Z = static_cast<uint8>(v & cpu.A) == 0;
  cpu.P.V = v.bit(6);
  cpu.P.N = v.bit(7);
}

static void op_bmi(Cpu &cpu) { branch(cpu, cpu.P.N); }

static void op_bne(Cpu &cpu) { branch(cpu, !cpu.P.Z); }

static void op_bpl(Cpu &cpu) { branch(cpu, !cpu.P.N); }

static void op_brk(Cpu &cpu) {
  // Dummy read
  // TODO: other instructions can cause dummy reads (PHP, PLP, etc.)?
  cpu.read(cpu.PC++);
  cpu.push16(cpu.PC);
  cpu.push(cpu.P | 0x30);
  cpu.PC = cpu.read16(0xfffe);
  cpu.P.I = true;
}

static void op_bvc(Cpu &cpu) { branch(cpu, !cpu.P.V); }

static void op_bvs(Cpu &cpu) { branch(cpu, cpu.P.V); }

static void op_clc(Cpu &cpu) {
  cpu.tick();
  cpu.P.C = false;
}

static void op_cld(Cpu &cpu) {
  cpu.tick();
  cpu.P.D = false;
}

static void op_cli(Cpu &cpu) {
  cpu.tick();
  cpu.P.I = false;
}

static void op_clv(Cpu &cpu) {
  cpu.tick();
  cpu.P.V = false;
}

template <addr_func_t T> static void op_cmp(Cpu &cpu) {
  const auto v = cpu.read(T(cpu));
  const auto r = static_cast<uint8>(cpu.A - v);

  cpu.P.C = cpu.A >= v;
  cpu.P.Z = (r == 0);
  cpu.P.N = r.bit(7);
}

template <addr_func_t T> static void op_cpx(Cpu &cpu) {
  const auto v = cpu.read(T(cpu));
  const auto r = static_cast<uint8>(cpu.X - v);

  cpu.P.C = cpu.X >= v;
  cpu.P.Z = (r == 0);
  cpu.P.N = r.bit(7);
}

template <addr_func_t T> static void op_cpy(Cpu &cpu) {
  const auto v = cpu.read(T(cpu));
  const auto r = static_cast<uint8>(cpu.Y - v);

  cpu.P.C = cpu.Y >= v;
  cpu.P.Z = (r == 0);
  cpu.P.N = r.bit(7);
}

template <addr_func_t T> static void op_dec(Cpu &cpu) {
  const auto a = T(cpu);
  const auto v = static_cast<uint8>(cpu.read(a) - 1);

  cpu.tick();

  cpu.P.Z = (v == 0);
  cpu.P.N = v.bit(7);

  cpu.write(a, v);
}

static void op_dex(Cpu &cpu) {
  cpu.tick();
  --cpu.X;

  cpu.P.Z = (cpu.X == 0);
  cpu.P.N = cpu.X.bit(7);
}

static void op_dey(Cpu &cpu) {
  cpu.tick();
  --cpu.Y;

  cpu.P.Z = (cpu.Y == 0);
  cpu.P.N = cpu.Y.bit(7);
}

template <addr_func_t T> static void op_eor(Cpu &cpu) {
  cpu.A ^= cpu.read(T(cpu));

  cpu.P.Z = cpu.A == 0;
  cpu.P.N = cpu.A.bit(7);
}

template <addr_func_t T> static void op_inc(Cpu &cpu) {
  const auto a = T(cpu);
  const auto v = static_cast<uint8>(cpu.read(a) + 1);

  cpu.tick();

  cpu.P.Z = (v == 0);
  cpu.P.N = v.bit(7);

  cpu.write(a, v);
}

static void op_inx(Cpu &cpu) {
  cpu.tick();
  ++cpu.X;

  cpu.P.Z = (cpu.X == 0);
  cpu.P.N = cpu.X.bit(7);
}

static void op_iny(Cpu &cpu) {
  cpu.tick();
  ++cpu.Y;

  cpu.P.Z = (cpu.Y == 0);
  cpu.P.N = cpu.Y.bit(7);
}

template <addr_func_t T> static void op_jmp(Cpu &cpu) { cpu.PC = T(cpu); }

static void op_jsr(Cpu &cpu) {
  cpu.tick();
  cpu.push16(cpu.PC + 1);
  cpu.PC = addr_abs(cpu);
}

template <addr_func_t T> static void op_lda(Cpu &cpu) {
  cpu.A = cpu.read(T(cpu));
  cpu.P.Z = (cpu.A == 0);
  cpu.P.N = cpu.A.bit(7);
}

template <addr_func_t T> static void op_ldx(Cpu &cpu) {
  cpu.X = cpu.read(T(cpu));

  cpu.P.Z = (cpu.X == 0);
  cpu.P.N = cpu.X.bit(7);
}

template <addr_func_t T> static void op_ldy(Cpu &cpu) {
  cpu.Y = cpu.read(T(cpu));

  cpu.P.Z = (cpu.Y == 0);
  cpu.P.N = cpu.Y.bit(7);
}

template <addr_func_t T> static void op_lsr(Cpu &cpu) {
  cpu.tick();

  if (T == addr_acc) {
    const auto v = cpu.A;
    const auto r = static_cast<uint8>(v >> 1);

    cpu.P.C = v.bit(0);
    cpu.P.Z = r == 0;
    cpu.P.N = false;

    cpu.A = r;
  } else {
    const auto address = T(cpu);
    const auto v = cpu.read(address);
    const auto r = static_cast<uint8>(v >> 1);

    cpu.P.C = v.bit(0);
    cpu.P.Z = r == 0;
    cpu.P.N = false;

    cpu.write(address, r);
  }
}

static void op_nop(Cpu &cpu) { cpu.tick(); }

template <addr_func_t T> static void op_ora(Cpu &cpu) {
  cpu.A |= cpu.read(T(cpu));

  cpu.P.Z = cpu.A == 0;
  cpu.P.N = cpu.A.bit(7);
}

static void op_pha(Cpu &cpu) {
  cpu.tick();
  cpu.push(cpu.A);
}

static void op_php(Cpu &cpu) {
  cpu.tick();
  cpu.push(cpu.P | 0x30);
}

static void op_pla(Cpu &cpu) {
  cpu.tick();
  cpu.tick();
  cpu.A = cpu.pop();

  cpu.P.Z = (cpu.A == 0);
  cpu.P.N = cpu.A.bit(7);
}

static void op_plp(Cpu &cpu) {
  cpu.tick();
  cpu.tick();
  cpu.P = cpu.pop();
}

template <addr_func_t T> static void op_rol(Cpu &cpu) {
  cpu.tick();

  if (T == addr_acc) {
    const auto v = cpu.A;
    const auto r = static_cast<uint8>((v << 1) | cpu.P.C);

    cpu.P.C = v.bit(7);
    cpu.P.Z = r == 0;
    cpu.P.N = r.bit(7);

    cpu.A = r;
  } else {
    const auto address = T(cpu);
    const auto v = cpu.read(address);
    const auto r = static_cast<uint8>((v << 1) | cpu.P.C);

    cpu.P.C = v.bit(7);
    cpu.P.Z = r == 0;
    cpu.P.N = r.bit(7);

    cpu.write(address, r);
  }
}

template <addr_func_t T> static void op_ror(Cpu &cpu) {
  cpu.tick();

  if (T == addr_acc) {
    const auto v = cpu.A;
    const auto r = static_cast<uint8>((cpu.P.C << 7) | (v >> 1));

    cpu.P.C = v.bit(0);
    cpu.P.Z = r == 0;
    cpu.P.N = r.bit(7);

    cpu.A = r;
  } else {
    const auto address = T(cpu);
    const auto v = cpu.read(address);
    const auto r = static_cast<uint8>((cpu.P.C << 7) | (v >> 1));

    cpu.P.C = v.bit(0);
    cpu.P.Z = r == 0;
    cpu.P.N = r.bit(7);

    cpu.write(address, r);
  }
}

static void op_rti(Cpu &cpu) {
  op_plp(cpu);
  cpu.PC = cpu.pop16();
}

static void op_rts(Cpu &cpu) {
  cpu.tick();
  cpu.tick();
  cpu.tick();
  cpu.PC = cpu.pop16() + 1;
}

template <addr_func_t T> static void op_sbc(Cpu &cpu) {
  const auto v = static_cast<uint8>(cpu.read(T(cpu)) ^ 0xff);
  const uint16 r16 = cpu.A + v + cpu.P.C;
  const auto r = static_cast<uint8>(r16);

  cpu.P.C = r16.bit(8);
  cpu.P.Z = r == 0;
  cpu.P.V = (~(cpu.A ^ v) & (cpu.A ^ r) & 0x80) != 0;
  cpu.P.N = r.bit(7);

  cpu.A = r;
}

static void op_sec(Cpu &cpu) {
  cpu.tick();
  cpu.P.C = true;
}

static void op_sed(Cpu &cpu) {
  cpu.tick();
  cpu.P.D = true;
}

static void op_sei(Cpu &cpu) {
  cpu.tick();
  cpu.P.I = true;
}

template <addr_func_t T> static void op_sta(Cpu &cpu) { cpu.write(T(cpu), cpu.A); }

template <addr_func_t T> static void op_stx(Cpu &cpu) { cpu.write(T(cpu), cpu.X); }

template <addr_func_t T> static void op_sty(Cpu &cpu) { cpu.write(T(cpu), cpu.Y); }

static void op_tax(Cpu &cpu) {
  cpu.tick();
  cpu.X = cpu.A;

  cpu.P.Z = (cpu.X == 0);
  cpu.P.N = cpu.X.bit(7);
}

static void op_tay(Cpu &cpu) {
  cpu.tick();
  cpu.Y = cpu.A;

  cpu.P.Z = (cpu.Y == 0);
  cpu.P.N = cpu.Y.bit(7);
}

static void op_tsx(Cpu &cpu) {
  cpu.tick();
  cpu.X = cpu.S;

  cpu.P.Z = (cpu.X == 0);
  cpu.P.N = cpu.X.bit(7);
}

static void op_txa(Cpu &cpu) {
  cpu.tick();
  cpu.A = cpu.X;

  cpu.P.Z = (cpu.A == 0);
  cpu.P.N = cpu.A.bit(7);
}

static void op_txs(Cpu &cpu) {
  cpu.tick();
  cpu.S = cpu.X;
}

static void op_tya(Cpu &cpu) {
  cpu.tick();
  cpu.A = cpu.Y;

  cpu.P.Z = (cpu.A == 0);
  cpu.P.N = cpu.A.bit(7);
}

const std::array<instr_func_t, 256> instructions = {
    // 0x00
    op_brk, op_ora<addr_inx>, op_nop, op_nop, op_nop, op_ora<addr_zpg>, op_asl<addr_zpg>, op_nop,
    op_php, op_ora<addr_imm>, op_asl<addr_acc>, op_nop, op_nop, op_ora<addr_abs>, op_asl<addr_abs>,
    op_nop,
    // 0x10
    op_bpl, op_ora<addr_iny>, op_nop, op_nop, op_nop, op_ora<addr_zpx>, op_asl<addr_zpx>, op_nop,
    op_clc, op_ora<addr_aby>, op_nop, op_nop, op_nop, op_ora<addr_abx>, op_asl<addr_abx<false>>,
    op_nop,
    // 0x20
    op_jsr, op_and<addr_inx>, op_nop, op_nop, op_bit<addr_zpg>, op_and<addr_zpg>, op_rol<addr_zpg>,
    op_nop, op_plp, op_and<addr_imm>, op_rol<addr_acc>, op_nop, op_bit<addr_abs>, op_and<addr_abs>,
    op_rol<addr_abs>, op_nop,
    // 0x30
    op_bmi, op_and<addr_iny>, op_nop, op_nop, op_nop, op_and<addr_zpx>, op_rol<addr_zpx>, op_nop,
    op_sec, op_and<addr_aby>, op_nop, op_nop, op_nop, op_and<addr_abx>, op_rol<addr_abx<false>>,
    op_nop,
    // 0x40
    op_rti, op_eor<addr_inx>, op_nop, op_nop, op_nop, op_eor<addr_zpg>, op_lsr<addr_zpg>, op_nop,
    op_pha, op_eor<addr_imm>, op_lsr<addr_acc>, op_nop, op_jmp<addr_abs>, op_eor<addr_abs>,
    op_lsr<addr_abs>, op_nop,
    // 0x50
    op_bvc, op_eor<addr_iny>, op_nop, op_nop, op_nop, op_eor<addr_zpx>, op_lsr<addr_zpx>, op_nop,
    op_cli, op_eor<addr_aby>, op_nop, op_nop, op_nop, op_eor<addr_abx>, op_lsr<addr_abx<false>>,
    op_nop,
    // 0x60
    op_rts, op_adc<addr_inx>, op_nop, op_nop, op_nop, op_adc<addr_zpg>, op_ror<addr_zpg>, op_nop,
    op_pla, op_adc<addr_imm>, op_ror<addr_acc>, op_nop, op_jmp<addr_ind>, op_adc<addr_abs>,
    op_ror<addr_abs>, op_nop,
    // 0x70
    op_bvs, op_adc<addr_iny>, op_nop, op_nop, op_nop, op_adc<addr_zpx>, op_ror<addr_zpx>, op_nop,
    op_sei, op_adc<addr_aby>, op_nop, op_nop, op_nop, op_adc<addr_abx>, op_ror<addr_abx<false>>,
    op_nop,
    // 0x80
    op_nop, op_sta<addr_inx>, op_nop, op_nop, op_sty<addr_zpg>, op_sta<addr_zpg>, op_stx<addr_zpg>,
    op_nop, op_dey, op_nop, op_txa, op_nop, op_sty<addr_abs>, op_sta<addr_abs>, op_stx<addr_abs>,
    op_nop,
    // 0x90
    op_bcc, op_sta<addr_iny>, op_nop, op_nop, op_sty<addr_zpx>, op_sta<addr_zpx>, op_stx<addr_zpy>,
    op_nop, op_tya, op_sta<addr_aby>, op_txs, op_nop, op_nop, op_sta<addr_abx>, op_nop, op_nop,
    // 0xa0
    op_ldy<addr_imm>, op_lda<addr_inx>, op_ldx<addr_imm>, op_nop, op_ldy<addr_zpg>,
    op_lda<addr_zpg>, op_ldx<addr_zpg>, op_nop, op_tay, op_lda<addr_imm>, op_tax, op_nop,
    op_ldy<addr_abs>, op_lda<addr_abs>, op_ldx<addr_abs>, op_nop,
    // 0xb0
    op_bcs, op_lda<addr_iny>, op_nop, op_nop, op_ldy<addr_zpx>, op_lda<addr_zpx>, op_ldx<addr_zpy>,
    op_nop, op_clv, op_lda<addr_aby>, op_tsx, op_nop, op_ldy<addr_abx>, op_lda<addr_abx>,
    op_ldx<addr_aby>, op_nop,
    // 0xc0
    op_cpy<addr_imm>, op_cmp<addr_inx>, op_nop, op_nop, op_cpy<addr_zpg>, op_cmp<addr_zpg>,
    op_dec<addr_zpg>, op_nop, op_iny, op_cmp<addr_imm>, op_dex, op_nop, op_cpy<addr_abs>,
    op_cmp<addr_abs>, op_dec<addr_abs>, op_nop,
    // 0xd0
    op_bne, op_cmp<addr_iny>, op_nop, op_nop, op_nop, op_cmp<addr_zpx>, op_dec<addr_zpx>, op_nop,
    op_cld, op_cmp<addr_aby>, op_nop, op_nop, op_nop, op_cmp<addr_abx>, op_dec<addr_abx<false>>,
    op_nop,
    // 0xe0
    op_cpx<addr_imm>, op_sbc<addr_inx>, op_nop, op_nop, op_cpx<addr_zpg>, op_sbc<addr_zpg>,
    op_inc<addr_zpg>, op_nop, op_inx, op_sbc<addr_imm>, op_nop, op_nop, op_cpx<addr_abs>,
    op_sbc<addr_abs>, op_inc<addr_abs>, op_nop,
    // 0xf0
    op_beq, op_sbc<addr_iny>, op_nop, op_nop, op_nop, op_sbc<addr_zpx>, op_inc<addr_zpx>, op_nop,
    op_sed, op_sbc<addr_aby>, op_nop, op_nop, op_nop, op_sbc<addr_abx>, op_inc<addr_abx<false>>,
    op_nop};

} // namespace

} // namespace nesturbia
