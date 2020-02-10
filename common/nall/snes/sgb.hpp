#ifndef NALL_SNES_SGB_HPP
#define NALL_SNES_SGB_HPP

namespace nall {

struct GBCPU {
  enum : unsigned {
    Implied,        //
    Imm8,           // $nn
    ImmS8,          // +/- $nn
    Imm16,          // $nnnn
    Addr8Ptr,       // ($ffnn)
    Addr16Ptr,      // ($nnnn)
    Addr16,         // $nnnn
    RegA,           // A
    RegAF,          // AF
    RegB,           // B
    RegBC,          // BC
    RegBCPtr,       // (BC)
    RegC,           // C
    RegCPtr,        // ($ff00+C)
    RegD,           // D
    RegDE,          // DE
    RegDEPtr,       // (DE)
    RegE,           // E
    RegH,           // H
    RegL,           // L
    RegHL,          // HL
    RegHLPtr,       // (HL)
    RegHLPtrInc,    // (HL+)
    RegHLPtrDec,    // (HL-)
    RegSP,          // SP
    FlagC,          // C
    FlagNC,         // NC
    FlagZ,          // Z
    FlagNZ,         // NZ
    PCRelative,     // PC+$nn
    SPRelative,     // SP+$nn
    RST,
    PrefixCB,
    Invalid,        // invalid opcodes
  };

  struct OpcodeInfo {
    char name[5];
    unsigned mode0;
    unsigned mode1;
  };
  
  static unsigned getOpcodeLength(uint8_t opcode);
  static bool getOpcodeIndirect(uint8_t opcode, uint8_t pl);
  static string disassemble(unsigned pc, uint8_t opcode, uint8_t pl, uint8_t ph);

private:
  static unsigned getModeLength(unsigned mode);
  static bool getModeIndirect(unsigned mode, uint8_t pl = 0);
  static string disassembleMode(unsigned pc, unsigned mode, uint8_t opcode = 0, uint8_t pl = 0, uint8_t ph = 0);
};

static const GBCPU::OpcodeInfo cpuOpcodeInfo[256] = {
  //0x00 - 0x0f
  {"nop " },
  {"ld  ", GBCPU::RegBC, GBCPU::Imm16 },
  {"ld  ", GBCPU::RegBCPtr, GBCPU::RegA },
  {"inc ", GBCPU::RegBC },
  {"inc ", GBCPU::RegB },
  {"dec ", GBCPU::RegB },
  {"ld  ", GBCPU::RegB, GBCPU::Imm8 },
  {"rlca" },
  {"ld  ", GBCPU::Addr16Ptr, GBCPU::RegSP },
  {"add ", GBCPU::RegHL, GBCPU::RegBC },
  {"ld  ", GBCPU::RegA, GBCPU::RegBCPtr },
  {"dec ", GBCPU::RegBC },
  {"inc ", GBCPU::RegC },
  {"dec ", GBCPU::RegC },
  {"ld  ", GBCPU::RegC, GBCPU::Imm8 },
  {"rrca" },
  
  //0x10 - 0x1f
  {"stop", GBCPU::Imm8 },
  {"ld  ", GBCPU::RegDE, GBCPU::Imm16 },
  {"ld  ", GBCPU::RegDEPtr, GBCPU::RegA },
  {"inc ", GBCPU::RegDE },
  {"inc ", GBCPU::RegD },
  {"dec ", GBCPU::RegD },
  {"ld  ", GBCPU::RegD, GBCPU::Imm8 },
  {"rla " },
  {"jr  ", GBCPU::PCRelative },
  {"add ", GBCPU::RegHL, GBCPU::RegDE },
  {"ld  ", GBCPU::RegA, GBCPU::RegDEPtr },
  {"dec ", GBCPU::RegDE },
  {"inc ", GBCPU::RegE },
  {"dec ", GBCPU::RegE },
  {"ld  ", GBCPU::RegE, GBCPU::Imm8 },
  {"rra " },
  
  //0x20 - 0x2f
  {"jr  ", GBCPU::FlagNZ, GBCPU::PCRelative },
  {"ld  ", GBCPU::RegHL, GBCPU::Imm16 },
  {"ld  ", GBCPU::RegHLPtrInc, GBCPU::RegA },
  {"inc ", GBCPU::RegHL },
  {"inc ", GBCPU::RegH },
  {"dec ", GBCPU::RegH },
  {"ld  ", GBCPU::RegH, GBCPU::Imm8 },
  {"daa " },
  {"jr  ", GBCPU::FlagZ, GBCPU::PCRelative },
  {"add ", GBCPU::RegHL, GBCPU::RegHL },
  {"ld  ", GBCPU::RegA, GBCPU::RegHLPtrInc },
  {"dec ", GBCPU::RegHL },
  {"inc ", GBCPU::RegL },
  {"dec ", GBCPU::RegL },
  {"ld  ", GBCPU::RegL, GBCPU::Imm8 },
  {"cpl " },
  
  //0x30 - 0x3f
  {"jr  ", GBCPU::FlagNZ, GBCPU::PCRelative },
  {"ld  ", GBCPU::RegSP, GBCPU::Imm16 },
  {"ld  ", GBCPU::RegHLPtrDec, GBCPU::RegA },
  {"inc ", GBCPU::RegSP },
  {"inc ", GBCPU::RegHLPtr },
  {"dec ", GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::Imm8 },
  {"scf " },
  {"jr  ", GBCPU::FlagC, GBCPU::PCRelative },
  {"add ", GBCPU::RegHL, GBCPU::RegSP },
  {"ld  ", GBCPU::RegA, GBCPU::RegHLPtrDec },
  {"dec ", GBCPU::RegSP },
  {"inc ", GBCPU::RegA },
  {"dec ", GBCPU::RegA },
  {"ld  ", GBCPU::RegA, GBCPU::Imm8 },
  {"ccf " },
  
  //0x40 - 0x4f
  {"ld  ", GBCPU::RegB, GBCPU::RegB },
  {"ld  ", GBCPU::RegB, GBCPU::RegC },
  {"ld  ", GBCPU::RegB, GBCPU::RegD },
  {"ld  ", GBCPU::RegB, GBCPU::RegE },
  {"ld  ", GBCPU::RegB, GBCPU::RegH },
  {"ld  ", GBCPU::RegB, GBCPU::RegL },
  {"ld  ", GBCPU::RegB, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegB, GBCPU::RegA },
  {"ld  ", GBCPU::RegC, GBCPU::RegB },
  {"ld  ", GBCPU::RegC, GBCPU::RegC },
  {"ld  ", GBCPU::RegC, GBCPU::RegD },
  {"ld  ", GBCPU::RegC, GBCPU::RegE },
  {"ld  ", GBCPU::RegC, GBCPU::RegH },
  {"ld  ", GBCPU::RegC, GBCPU::RegL },
  {"ld  ", GBCPU::RegC, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegC, GBCPU::RegA },
  
  //0x50 - 0x5f
  {"ld  ", GBCPU::RegD, GBCPU::RegB },
  {"ld  ", GBCPU::RegD, GBCPU::RegC },
  {"ld  ", GBCPU::RegD, GBCPU::RegD },
  {"ld  ", GBCPU::RegD, GBCPU::RegE },
  {"ld  ", GBCPU::RegD, GBCPU::RegH },
  {"ld  ", GBCPU::RegD, GBCPU::RegL },
  {"ld  ", GBCPU::RegD, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegD, GBCPU::RegA },
  {"ld  ", GBCPU::RegE, GBCPU::RegB },
  {"ld  ", GBCPU::RegE, GBCPU::RegC },
  {"ld  ", GBCPU::RegE, GBCPU::RegD },
  {"ld  ", GBCPU::RegE, GBCPU::RegE },
  {"ld  ", GBCPU::RegE, GBCPU::RegH },
  {"ld  ", GBCPU::RegE, GBCPU::RegL },
  {"ld  ", GBCPU::RegE, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegE, GBCPU::RegA },
  
  //0x60 - 0x6f
  {"ld  ", GBCPU::RegH, GBCPU::RegB },
  {"ld  ", GBCPU::RegH, GBCPU::RegC },
  {"ld  ", GBCPU::RegH, GBCPU::RegD },
  {"ld  ", GBCPU::RegH, GBCPU::RegE },
  {"ld  ", GBCPU::RegH, GBCPU::RegH },
  {"ld  ", GBCPU::RegH, GBCPU::RegL },
  {"ld  ", GBCPU::RegH, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegH, GBCPU::RegA },
  {"ld  ", GBCPU::RegL, GBCPU::RegB },
  {"ld  ", GBCPU::RegL, GBCPU::RegC },
  {"ld  ", GBCPU::RegL, GBCPU::RegD },
  {"ld  ", GBCPU::RegL, GBCPU::RegE },
  {"ld  ", GBCPU::RegL, GBCPU::RegH },
  {"ld  ", GBCPU::RegL, GBCPU::RegL },
  {"ld  ", GBCPU::RegL, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegL, GBCPU::RegA },
  
  //0x70 - 0x7f
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegB },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegC },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegD },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegE },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegH },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegL },
  {"halt" },
  {"ld  ", GBCPU::RegHLPtr, GBCPU::RegA },
  {"ld  ", GBCPU::RegA, GBCPU::RegB },
  {"ld  ", GBCPU::RegA, GBCPU::RegC },
  {"ld  ", GBCPU::RegA, GBCPU::RegD },
  {"ld  ", GBCPU::RegA, GBCPU::RegE },
  {"ld  ", GBCPU::RegA, GBCPU::RegH },
  {"ld  ", GBCPU::RegA, GBCPU::RegL },
  {"ld  ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"ld  ", GBCPU::RegA, GBCPU::RegA },
  
  //0x80 - 0x8f
  {"add ", GBCPU::RegA, GBCPU::RegB },
  {"add ", GBCPU::RegA, GBCPU::RegC },
  {"add ", GBCPU::RegA, GBCPU::RegD },
  {"add ", GBCPU::RegA, GBCPU::RegE },
  {"add ", GBCPU::RegA, GBCPU::RegH },
  {"add ", GBCPU::RegA, GBCPU::RegL },
  {"add ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"add ", GBCPU::RegA, GBCPU::RegA },
  {"adc ", GBCPU::RegA, GBCPU::RegB },
  {"adc ", GBCPU::RegA, GBCPU::RegC },
  {"adc ", GBCPU::RegA, GBCPU::RegD },
  {"adc ", GBCPU::RegA, GBCPU::RegE },
  {"adc ", GBCPU::RegA, GBCPU::RegH },
  {"adc ", GBCPU::RegA, GBCPU::RegL },
  {"adc ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"adc ", GBCPU::RegA, GBCPU::RegA },
  
  //0x90 - 0x9f
  {"sub ", GBCPU::RegA, GBCPU::RegB },
  {"sub ", GBCPU::RegA, GBCPU::RegC },
  {"sub ", GBCPU::RegA, GBCPU::RegD },
  {"sub ", GBCPU::RegA, GBCPU::RegE },
  {"sub ", GBCPU::RegA, GBCPU::RegH },
  {"sub ", GBCPU::RegA, GBCPU::RegL },
  {"sub ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"sub ", GBCPU::RegA, GBCPU::RegA },
  {"sbc ", GBCPU::RegA, GBCPU::RegB },
  {"sbc ", GBCPU::RegA, GBCPU::RegC },
  {"sbc ", GBCPU::RegA, GBCPU::RegD },
  {"sbc ", GBCPU::RegA, GBCPU::RegE },
  {"sbc ", GBCPU::RegA, GBCPU::RegH },
  {"sbc ", GBCPU::RegA, GBCPU::RegL },
  {"sbc ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"sbc ", GBCPU::RegA, GBCPU::RegA },
  
  //0xa0 - 0xaf
  {"and ", GBCPU::RegA, GBCPU::RegB },
  {"and ", GBCPU::RegA, GBCPU::RegC },
  {"and ", GBCPU::RegA, GBCPU::RegD },
  {"and ", GBCPU::RegA, GBCPU::RegE },
  {"and ", GBCPU::RegA, GBCPU::RegH },
  {"and ", GBCPU::RegA, GBCPU::RegL },
  {"and ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"and ", GBCPU::RegA, GBCPU::RegA },
  {"xor ", GBCPU::RegA, GBCPU::RegB },
  {"xor ", GBCPU::RegA, GBCPU::RegC },
  {"xor ", GBCPU::RegA, GBCPU::RegD },
  {"xor ", GBCPU::RegA, GBCPU::RegE },
  {"xor ", GBCPU::RegA, GBCPU::RegH },
  {"xor ", GBCPU::RegA, GBCPU::RegL },
  {"xor ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"xor ", GBCPU::RegA, GBCPU::RegA },
  
  //0xb0 - 0xbf
  {"or  ", GBCPU::RegA, GBCPU::RegB },
  {"or  ", GBCPU::RegA, GBCPU::RegC },
  {"or  ", GBCPU::RegA, GBCPU::RegD },
  {"or  ", GBCPU::RegA, GBCPU::RegE },
  {"or  ", GBCPU::RegA, GBCPU::RegH },
  {"or  ", GBCPU::RegA, GBCPU::RegL },
  {"or  ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"or  ", GBCPU::RegA, GBCPU::RegA },
  {"cp  ", GBCPU::RegA, GBCPU::RegB },
  {"cp  ", GBCPU::RegA, GBCPU::RegC },
  {"cp  ", GBCPU::RegA, GBCPU::RegD },
  {"cp  ", GBCPU::RegA, GBCPU::RegE },
  {"cp  ", GBCPU::RegA, GBCPU::RegH },
  {"cp  ", GBCPU::RegA, GBCPU::RegL },
  {"cp  ", GBCPU::RegA, GBCPU::RegHLPtr },
  {"cp  ", GBCPU::RegA, GBCPU::RegA },
  
  //0xc0 - 0xcf
  {"ret ", GBCPU::FlagNZ },
  {"pop ", GBCPU::RegBC },
  {"jp  ", GBCPU::FlagNZ, GBCPU::Addr16 },
  {"jp  ", GBCPU::Addr16 },
  {"call", GBCPU::FlagNZ, GBCPU::Addr16 },
  {"push", GBCPU::RegBC },
  {"add ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  {"ret ", GBCPU::FlagZ },
  {"ret " },
  {"jp  ", GBCPU::FlagZ, GBCPU::Addr16 },
  {"", GBCPU::PrefixCB }, // special case
  {"call", GBCPU::FlagZ, GBCPU::Addr16 },
  {"call", GBCPU::Addr16 },
  {"adc ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  
  //0xd0 - 0xdf
  {"ret ", GBCPU::FlagNC },
  {"pop ", GBCPU::RegDE },
  {"jp  ", GBCPU::FlagNC, GBCPU::Addr16 },
  {"????", GBCPU::Invalid },
  {"call", GBCPU::FlagNC, GBCPU::Addr16 },
  {"push", GBCPU::RegDE },
  {"sub ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  {"ret ", GBCPU::FlagC },
  {"reti" },
  {"jp  ", GBCPU::FlagC, GBCPU::Addr16 },
  {"????", GBCPU::Invalid },
  {"call", GBCPU::FlagC, GBCPU::Addr16 },
  {"????", GBCPU::Invalid },
  {"sbc ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  
  //0xe0 - 0xef
  {"ld  ", GBCPU::Addr8Ptr, GBCPU::RegA },
  {"pop ", GBCPU::RegHL },
  {"ld  ", GBCPU::RegCPtr, GBCPU::RegA },
  {"????", GBCPU::Invalid },
  {"????", GBCPU::Invalid },
  {"push", GBCPU::RegHL },
  {"and ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  {"add ", GBCPU::RegSP, GBCPU::ImmS8 },
  {"jp  ", GBCPU::RegHLPtr },
  {"ld  ", GBCPU::Addr16Ptr, GBCPU::RegA },
  {"????", GBCPU::Invalid },
  {"????", GBCPU::Invalid },
  {"????", GBCPU::Invalid },
  {"xor ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  
  //0xf0 - 0xff
  {"ld  ", GBCPU::RegA, GBCPU::Addr8Ptr },
  {"pop ", GBCPU::RegAF },
  {"ld  ", GBCPU::RegA, GBCPU::RegCPtr },
  {"di  " },
  {"????", GBCPU::Invalid },
  {"push", GBCPU::RegAF },
  {"or  ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
  {"ld  ", GBCPU::RegHL, GBCPU::SPRelative },
  {"ld  ", GBCPU::RegSP, GBCPU::RegHL },
  {"ld  ", GBCPU::RegA, GBCPU::Addr16Ptr },
  {"ei  " },
  {"????", GBCPU::Invalid },
  {"????", GBCPU::Invalid },
  {"cp  ", GBCPU::RegA, GBCPU::Imm8 },
  {"rst ", GBCPU::RST },
};

static const char cpuPrefixName[32][5] = {
  "rlc ",
  "rrc ",
  "rl  ",
  "rr  ",
  "sla ",
  "sra ",
  "swap",
  "srl ",
  "bit ",
  "bit ",
  "bit ",
  "bit ",
  "bit ",
  "bit ",
  "bit ",
  "bit ",
  "res ",
  "res ",
  "res ",
  "res ",
  "res ",
  "res ",
  "res ",
  "res ",
  "set ",
  "set ",
  "set ",
  "set ",
  "set ",
  "set ",
  "set ",
  "set "
};

static const unsigned cpuPrefixMode[8] = {
  GBCPU::RegB,
  GBCPU::RegC,
  GBCPU::RegD,
  GBCPU::RegE,
  GBCPU::RegH,
  GBCPU::RegL,
  GBCPU::RegHLPtr,
  GBCPU::RegA
};

inline unsigned GBCPU::getModeLength(unsigned mode) {
  switch (mode) { 
    default:
      return 0;
    case Imm8:
    case ImmS8:
    case Addr8Ptr:
    case PCRelative:
    case SPRelative:
    case PrefixCB:
      return 1;
    case Imm16:
    case Addr16Ptr:
    case Addr16:
      return 2;
  }
}

inline unsigned GBCPU::getOpcodeLength(uint8_t opcode) {
  const OpcodeInfo &op = cpuOpcodeInfo[opcode];
  return 1 + getModeLength(op.mode0) + getModeLength(op.mode1);
}

inline bool GBCPU::getModeIndirect(unsigned mode, uint8_t pl) {
  switch (mode) { 
    default:
      return false;
    case Addr8Ptr:
    case Addr16Ptr:
    case RegBCPtr:
    case RegCPtr:
    case RegHLPtr:
    case RegHLPtrInc:
    case RegHLPtrDec:
      return true;
    case PrefixCB:
      return getModeIndirect(cpuPrefixMode[pl & 0x7]);
  }
}

inline bool GBCPU::getOpcodeIndirect(uint8_t opcode, uint8_t pl) {
  const OpcodeInfo &op = cpuOpcodeInfo[opcode];
  return getModeIndirect(op.mode0, pl) || getModeIndirect(op.mode1, pl);
}

inline string GBCPU::disassembleMode(unsigned pc, unsigned mode, uint8_t opcode, uint8_t pl, uint8_t ph) {
  switch (mode) {
  default:          return "";
  case Imm8:        return { " $", hex<2>(pl) };
  case ImmS8:       return { " ", integer((int8_t)pl) };
  case Imm16:
  case Addr16:      return { " $", hex<2>(ph), hex<2>(pl) };
  case Addr8Ptr:    return { " ($ff", hex<2>(pl), ")" };
  case Addr16Ptr:   return { " ($", hex<2>(ph), hex<2>(pl), ")" };
  case RegA:        return " a";
  case RegAF:       return " af";
  case RegB:        return " b";
  case RegBC:       return " bc";
  case RegBCPtr:    return " (bc)";
  case RegC:        return " c";
  case RegCPtr:     return " ($ff00+c)";
  case RegD:        return " d";
  case RegDE:       return " de";
  case RegDEPtr:    return " (de)";
  case RegE:        return " e";
  case RegH:        return " h";
  case RegL:        return " l";
  case RegHL:       return " hl";
  case RegHLPtr:    return " (hl)";
  case RegHLPtrInc: return " (hl+)";
  case RegHLPtrDec: return " (hl-)";
  case RegSP:       return " sp";
  case FlagC:       return " c";
  case FlagNC:      return " nc";
  case FlagZ:       return " z";
  case FlagNZ:      return " nz";
  case RST:         return { " $", hex<2>(opcode & 0x38) };
  case PCRelative:  return { " $", hex<4>((uint16_t)(pc + 2 + (int8_t)pl)) };
  case SPRelative:
    if (pl & 0x80)
      return { " sp", integer((int8_t)pl) };
    else
      return { " sp+", integer((int8_t)pl) };
  case PrefixCB:
    if (pl > 0x40)
      return { cpuPrefixName[pl >> 3], " ", ((pl & 0x38) >> 3), ",", disassembleMode(pc, cpuPrefixMode[pl & 0x7]) };
    else
      return { cpuPrefixName[pl >> 3], disassembleMode(pc, cpuPrefixMode[pl & 0x7]) };
  }
}

inline string GBCPU::disassemble(unsigned pc, uint8_t opcode, uint8_t pl, uint8_t ph) {
  const OpcodeInfo &op = cpuOpcodeInfo[opcode];

  if (op.mode1 == Implied) {
    return { op.name, disassembleMode(pc, op.mode0, opcode, pl, ph) };
  } else {
    return { op.name, disassembleMode(pc, op.mode0, opcode, pl, ph), ",", disassembleMode(pc, op.mode1, opcode, pl, ph) };
  }
  
  return "";
}

}

#endif
