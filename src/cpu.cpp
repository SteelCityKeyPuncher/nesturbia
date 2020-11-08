#include <array>
#include <utility>

#include "nesturbia/cpu.hpp"

namespace nesturbia {

namespace {

extern const std::array<void (*)(Cpu &), 256> instructions;

} // namespace

Cpu::Cpu(read_callback_t readCallback, write_callback_t writeCallback)
    : readCallback(std::move(readCallback)), writeCallback(std::move(writeCallback)) {}

void Cpu::Power() {
  A = 0x00;
  X = 0x00;
  Y = 0x00;
  S = 0xfd;
  PC = read16(0xfffc);
  P = 0x4;

  cycles = 7;
}

uint8 Cpu::read(uint16 address) {
  tick();

  if (!readCallback) {
    return 0xff;
  }

  return readCallback(address);
}

uint16 Cpu::read16(uint16 address) {
  return read(address) | (read(static_cast<uint16>(address + 1)) << 8);
}

void Cpu::write(uint16 address, uint8 value) {
  if (writeCallback) {
    writeCallback(address, value);
  }
}

void Cpu::write16(uint16 address, uint16 value) {
  writeCallback(address, static_cast<uint8>(value));
  writeCallback((uint16)(address + 1), static_cast<uint8>(value >> 8));
}

void Cpu::tick() { ++cycles; }

void Cpu::executeInstruction() {
  const auto opcode = read(PC++);
  instructions[opcode](*this);
}

namespace {

// Helper function
inline bool checkPageCross(uint16 value, int8_t offset) {
  return (((value + offset) & 0xff00) != (value & 0xff00));
}

// Addressing modes
using addr_func_t = uint16 (*)(Cpu &);

inline uint16 addr_abs(Cpu &cpu) { return cpu.read16((cpu.PC += 2) - 2); }

inline uint16 addr_abx(Cpu &cpu) {
  const auto v = addr_abs(cpu);
  const auto xSigned = static_cast<int8_t>(cpu.X);

  if (checkPageCross(v, xSigned)) {
    cpu.tick();
  }

  return v + xSigned;
}

inline uint16 addr_aby(Cpu &cpu) {
  const auto v = addr_abs(cpu);
  const auto ySigned = static_cast<int8_t>(cpu.Y);

  if (checkPageCross(v, ySigned)) {
    cpu.tick();
  }

  return v + ySigned;
}

inline uint16 addr_acc(Cpu &) { return 0; }

inline uint16 addr_imm(Cpu &cpu) { return cpu.PC++; }

inline uint16 addr_imp(Cpu &) { return 0; }

inline uint16 addr_ind(Cpu &) { return 0; }

inline uint16 addr_inx(Cpu &cpu) {
  const auto l = static_cast<uint8>(cpu.read(cpu.PC++) + cpu.X);
  const auto h = static_cast<uint8>(l + 1);
  cpu.tick();
  return cpu.read(l) | (cpu.read(h) << 8);
}

inline uint16 addr_iny(Cpu &cpu) {
  const auto l = cpu.read(cpu.PC++);
  const auto h = static_cast<uint8>(l + 1);
  const auto v = cpu.read(l) | cpu.read(h) << 8;
  const auto ySigned = static_cast<int8_t>(cpu.Y);

  if (checkPageCross(v, ySigned)) {
    cpu.tick();
  }

  return v + ySigned;
}

inline uint16 addr_rel(Cpu &) { return 0; }

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

static void op_nop(Cpu &) {}

const std::array<instr_func_t, 256> instructions = {
    // 0x00
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0x10
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0x20
    op_nop, op_and<addr_inx>, op_nop, op_nop, op_nop, op_and<addr_zpg>, op_nop, op_nop, op_nop,
    op_and<addr_imm>, op_nop, op_nop, op_nop, op_and<addr_abs>, op_nop, op_nop,
    // 0x30
    op_nop, op_and<addr_iny>, op_nop, op_nop, op_nop, op_and<addr_zpx>, op_nop, op_nop, op_nop,
    op_and<addr_aby>, op_nop, op_nop, op_nop, op_and<addr_abx>, op_nop, op_nop,
    // 0x40
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0x50
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0x60
    op_nop, op_adc<addr_inx>, op_nop, op_nop, op_nop, op_adc<addr_zpg>, op_nop, op_nop, op_nop,
    op_adc<addr_imm>, op_nop, op_nop, op_nop, op_adc<addr_abs>, op_nop, op_nop,
    // 0x70
    op_nop, op_adc<addr_iny>, op_nop, op_nop, op_nop, op_adc<addr_zpx>, op_nop, op_nop, op_nop,
    op_adc<addr_aby>, op_nop, op_nop, op_nop, op_adc<addr_abx>, op_nop, op_nop,
    // 0x80
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0x90
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0xa0
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0xb0
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0xc0
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0xd0
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0xe0
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop,
    // 0xf0
    op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop, op_nop,
    op_nop, op_nop, op_nop, op_nop};

} // namespace

} // namespace nesturbia
