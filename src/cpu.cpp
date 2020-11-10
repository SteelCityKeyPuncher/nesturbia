#include <array>
#include <utility>

#include "nesturbia/cpu.hpp"

namespace nesturbia {

namespace {

extern const std::array<void (*)(Cpu &), 256> instructions;

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
  tick();

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
}

void Cpu::executeInstruction() {
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
      cpu.tick();
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
    cpu.tick();
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
  ++cpu.PC;
  cpu.push16(cpu.PC);
  cpu.push(cpu.P | 0x10);
  cpu.PC = cpu.read16(0xfffe);
  cpu.P.I = true;
  cpu.tick();
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
