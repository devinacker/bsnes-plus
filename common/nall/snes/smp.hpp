#ifndef NALL_SNES_SMP_HPP
#define NALL_SNES_SMP_HPP

namespace nall {

struct SNESSMP {
  enum : unsigned {
    Implied,          //
    TVector,          //0
    Direct,           //$00
    DirectRelative,   //$00,+/-$00
    ADirect,          //a,$00
    AAbsolute,        //a,$0000
    AIX,              //a,(x)
    AIDirectIndX,     //a,($00+x)
    AConstant,        //a,#$00
    DirectDirect,     //$00,$00
    CAbsoluteBit,     //c,$0000:0
    Absolute,         //$0000
    P,                //p
    AbsoluteA,        //$0000,a
    Relative,         //+/-$00
    ADirectIndX,      //a,$00+x
    AAbsoluteX,       //a,$0000+x
    AAbsoluteY,       //a,$0000+y
    AIDirectIndY,     //a,($00)+y
    DirectConstant,   //$00,#$00
    IXIY,             //(x),(y)
    DirectIndX,       //$00+x
    A,                //a
    X,                //x
    XAbsolute,        //x,$0000
    IAbsoluteX,       //($0000+x)
    CNAbsoluteBit,    //c,!$0000:0
    XDirect,          //x,$00
    PVector,          //$ff00
    YaDirect,         //ya,$00
    XA,               //x,a
    YAbsolute,        //y,$0000
    Y,                //y
    AX,               //a,x
    YDirect,          //y,$00
    YConstant,        //y,#$00
    XSp,              //x,sp
    YaX,              //ya,x
    IXPA,             //(x)+,a
    SpX,              //sp,x
    AIXP,             //a,(x)+
    DirectA,          //$00,a
    IXA,              //(x),a
    IDirectIndXA,     //($00+x),a
    XConstant,        //x,#$00
    AbsoluteX,        //$0000,x
    AbsoluteBitC,     //$0000:0,c
    DirectY,          //$00,y
    AbsoluteY,        //$0000,y
    Ya,               //ya
    DirectIndXA,      //$00+x,a
    AbsoluteXA,       //$0000+x,a
    AbsoluteYA,       //$0000+y,a
    IDirectIndYA,     //($00)+y,a
	DirectX,          //$00,x
    DirectIndYX,      //$00+y,x
    DirectYa,         //$00,ya
    DirectIndXY,      //$00+x,y
    AY,               //a,y
    DirectIndXRelative,//$00+x,+/-$00
    XDirectIndY,      //x,$00+y
    YDirectIndX,      //y,$00+x
    YA,               //y,a
    YRelative,        //y,+/-$00
  };

  struct OpcodeInfo {
    char name[6];
    unsigned mode;
  };

  static unsigned getOpcodeLength(uint8_t opcode);
  static bool getOpcodeIndirect(uint8_t opcode);
  static string disassemble(uint16_t pc, uint8_t opcode, uint8_t pl, uint8_t ph, bool p = false);
};

static const SNESSMP::OpcodeInfo smpOpcodeInfo[256] = {
  //0x00 - 0x0f
  { "nop  ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set0 ", SNESSMP::Direct },
  { "bbs0 ", SNESSMP::DirectRelative },

  { "or   ", SNESSMP::ADirect },
  { "or   ", SNESSMP::AAbsolute },
  { "or   ", SNESSMP::AIX },
  { "or   ", SNESSMP::AIDirectIndX },

  { "or   ", SNESSMP::AConstant },
  { "or   ", SNESSMP::DirectDirect },
  { "or1  ", SNESSMP::CAbsoluteBit },
  { "asl  ", SNESSMP::Direct },

  { "asl  ", SNESSMP::Absolute },
  { "push ", SNESSMP::P },
  { "tset ", SNESSMP::AbsoluteA },
  { "brk  ", SNESSMP::Implied },

  //0x10 - 0x1f
  { "bpl  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr0 ", SNESSMP::Direct },
  { "bbc0 ", SNESSMP::DirectRelative },

  { "or   ", SNESSMP::ADirectIndX },
  { "or   ", SNESSMP::AAbsoluteX },
  { "or   ", SNESSMP::AAbsoluteY },
  { "or   ", SNESSMP::AIDirectIndY },

  { "or   ", SNESSMP::DirectConstant },
  { "or   ", SNESSMP::IXIY },
  { "decw ", SNESSMP::Direct },
  { "asl  ", SNESSMP::DirectIndX },

  { "asl  ", SNESSMP::A },
  { "dec  ", SNESSMP::X },
  { "cmp  ", SNESSMP::XAbsolute },
  { "jmp  ", SNESSMP::IAbsoluteX },

  //0x20 - 0x2f
  { "clrp ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set1 ", SNESSMP::Direct },
  { "bbs1 ", SNESSMP::DirectRelative },

  { "and  ", SNESSMP::ADirect },
  { "and  ", SNESSMP::AAbsolute },
  { "and  ", SNESSMP::AIX },
  { "and  ", SNESSMP::AIDirectIndX },

  { "and  ", SNESSMP::AConstant },
  { "and  ", SNESSMP::DirectDirect },
  { "or1  ", SNESSMP::CNAbsoluteBit },
  { "rol  ", SNESSMP::Direct },

  { "rol  ", SNESSMP::Absolute },
  { "push ", SNESSMP::A },
  { "cbne ", SNESSMP::DirectRelative },
  { "bra  ", SNESSMP::Relative },

  //0x30 - 0x3f
  { "bmi  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr1 ", SNESSMP::Direct },
  { "bbc1 ", SNESSMP::DirectRelative },

  { "and  ", SNESSMP::ADirectIndX },
  { "and  ", SNESSMP::AAbsoluteX },
  { "and  ", SNESSMP::AAbsoluteY },
  { "and  ", SNESSMP::AIDirectIndY },

  { "and  ", SNESSMP::DirectConstant },
  { "and  ", SNESSMP::IXIY },
  { "incw ", SNESSMP::Direct },
  { "rol  ", SNESSMP::DirectIndX },

  { "rol  ", SNESSMP::A },
  { "inc  ", SNESSMP::X },
  { "cmp  ", SNESSMP::XDirect },
  { "call ", SNESSMP::Absolute },

  //0x40 - 0x4f
  { "setp ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set2 ", SNESSMP::Direct },
  { "bbs2 ", SNESSMP::DirectRelative },

  { "eor  ", SNESSMP::ADirect },
  { "eor  ", SNESSMP::AAbsolute },
  { "eor  ", SNESSMP::AIX },
  { "eor  ", SNESSMP::AIDirectIndX },

  { "eor  ", SNESSMP::AConstant },
  { "eor  ", SNESSMP::DirectDirect },
  { "and1 ", SNESSMP::CAbsoluteBit },
  { "lsr  ", SNESSMP::Direct },

  { "lsr  ", SNESSMP::Absolute },
  { "push ", SNESSMP::X },
  { "tclr ", SNESSMP::AbsoluteA },
  { "pcall", SNESSMP::PVector },

  //0x50 - 0x5f
  { "bvc  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr2 ", SNESSMP::Direct },
  { "bbc2 ", SNESSMP::DirectRelative },

  { "eor  ", SNESSMP::ADirectIndX },
  { "eor  ", SNESSMP::AAbsoluteX },
  { "eor  ", SNESSMP::AAbsoluteY },
  { "eor  ", SNESSMP::AIDirectIndY },

  { "eor  ", SNESSMP::DirectConstant },
  { "eor  ", SNESSMP::IXIY },
  { "cmpw ", SNESSMP::YaDirect },
  { "lsr  ", SNESSMP::DirectIndX },

  { "lsr  ", SNESSMP::A },
  { "mov  ", SNESSMP::XA },
  { "cmp  ", SNESSMP::YAbsolute },
  { "jmp  ", SNESSMP::Absolute },

  //0x60 - 0x6f
  { "clrc ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set3 ", SNESSMP::Direct },
  { "bbs3 ", SNESSMP::DirectRelative },

  { "cmp  ", SNESSMP::ADirect },
  { "cmp  ", SNESSMP::AAbsolute },
  { "cmp  ", SNESSMP::AIX },
  { "cmp  ", SNESSMP::AIDirectIndX },

  { "cmp  ", SNESSMP::AConstant },
  { "cmp  ", SNESSMP::DirectDirect },
  { "and1 ", SNESSMP::CNAbsoluteBit },
  { "ror  ", SNESSMP::Direct },

  { "ror  ", SNESSMP::Absolute },
  { "push ", SNESSMP::Y },
  { "dbnz ", SNESSMP::DirectRelative },
  { "ret  ", SNESSMP::Implied },

  //0x70 - 0x7f
  { "bvs  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr3 ", SNESSMP::Direct },
  { "bbc3 ", SNESSMP::DirectRelative },

  { "cmp  ", SNESSMP::ADirectIndX },
  { "cmp  ", SNESSMP::AAbsoluteX },
  { "cmp  ", SNESSMP::AAbsoluteY },
  { "cmp  ", SNESSMP::AIDirectIndY },

  { "cmp  ", SNESSMP::DirectConstant },
  { "cmp  ", SNESSMP::IXIY },
  { "addw ", SNESSMP::YaDirect },
  { "ror  ", SNESSMP::DirectIndX },

  { "ror  ", SNESSMP::A },
  { "mov  ", SNESSMP::AX },
  { "cmp  ", SNESSMP::YDirect },
  { "reti ", SNESSMP::Implied },

  //0x80 - 0x8f
  { "setc ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set4 ", SNESSMP::Direct },
  { "bbs4 ", SNESSMP::DirectRelative },

  { "adc  ", SNESSMP::ADirect },
  { "adc  ", SNESSMP::AAbsolute },
  { "adc  ", SNESSMP::AIX },
  { "adc  ", SNESSMP::AIDirectIndX },

  { "adc  ", SNESSMP::AConstant },
  { "adc  ", SNESSMP::DirectDirect },
  { "eor1 ", SNESSMP::CAbsoluteBit },
  { "dec  ", SNESSMP::Direct },

  { "dec  ", SNESSMP::Absolute },
  { "mov  ", SNESSMP::YConstant },
  { "pop  ", SNESSMP::P },
  { "mov  ", SNESSMP::DirectConstant },

  //0x90 - 0x9f
  { "bcc  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr4 ", SNESSMP::Direct },
  { "bbc4 ", SNESSMP::DirectRelative },

  { "adc  ", SNESSMP::ADirectIndX },
  { "adc  ", SNESSMP::AAbsoluteX },
  { "adc  ", SNESSMP::AAbsoluteY },
  { "adc  ", SNESSMP::AIDirectIndY },

  { "adc  ", SNESSMP::DirectConstant },
  { "adc  ", SNESSMP::IXIY },
  { "subw ", SNESSMP::YaDirect },
  { "dec  ", SNESSMP::DirectIndX },

  { "dec  ", SNESSMP::A },
  { "mov  ", SNESSMP::XSp },
  { "div  ", SNESSMP::YaX },
  { "xcn  ", SNESSMP::A },

  //0xa0 - 0xaf
  { "ei   ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set5 ", SNESSMP::Direct },
  { "bbs5 ", SNESSMP::DirectRelative },

  { "sbc  ", SNESSMP::ADirect },
  { "sbc  ", SNESSMP::AAbsolute },
  { "sbc  ", SNESSMP::AIX },
  { "sbc  ", SNESSMP::AIDirectIndX },

  { "sbc  ", SNESSMP::AConstant },
  { "sbc  ", SNESSMP::DirectDirect },
  { "mov1 ", SNESSMP::CAbsoluteBit },
  { "inc  ", SNESSMP::Direct },

  { "inc  ", SNESSMP::Absolute },
  { "cmp  ", SNESSMP::YConstant },
  { "pop  ", SNESSMP::A },
  { "mov  ", SNESSMP::IXPA },

  //0xb0 - 0xbf
  { "bcs  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr5 ", SNESSMP::Direct },
  { "bbc5 ", SNESSMP::DirectRelative },

  { "sbc  ", SNESSMP::ADirectIndX },
  { "sbc  ", SNESSMP::AAbsoluteX },
  { "sbc  ", SNESSMP::AAbsoluteY },
  { "sbc  ", SNESSMP::AIDirectIndY },

  { "sbc  ", SNESSMP::DirectConstant },
  { "sbc  ", SNESSMP::IXIY },
  { "movw ", SNESSMP::YaDirect },
  { "inc  ", SNESSMP::DirectIndX },

  { "inc  ", SNESSMP::A },
  { "mov  ", SNESSMP::SpX },
  { "das  ", SNESSMP::A },
  { "mov  ", SNESSMP::AIXP },

  //0xc0 - 0xcf
  { "di   ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set6 ", SNESSMP::Direct },
  { "bbs6 ", SNESSMP::DirectRelative },

  { "mov  ", SNESSMP::DirectA },
  { "mov  ", SNESSMP::AbsoluteA },
  { "mov  ", SNESSMP::IXA },
  { "mov  ", SNESSMP::IDirectIndXA },

  { "cmp  ", SNESSMP::XConstant },
  { "mov  ", SNESSMP::AbsoluteX },
  { "mov1 ", SNESSMP::AbsoluteBitC },
  { "mov  ", SNESSMP::DirectY },

  { "mov  ", SNESSMP::AbsoluteY },
  { "mov  ", SNESSMP::XConstant },
  { "pop  ", SNESSMP::X },
  { "mul  ", SNESSMP::Ya },

  //0xd0 - 0xdf
  { "bne  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr6 ", SNESSMP::Relative },
  { "bbc6 ", SNESSMP::DirectRelative },

  { "mov  ", SNESSMP::DirectIndXA },
  { "mov  ", SNESSMP::AbsoluteXA },
  { "mov  ", SNESSMP::AbsoluteYA },
  { "mov  ", SNESSMP::IDirectIndYA },

  { "mov  ", SNESSMP::DirectX },
  { "mov  ", SNESSMP::DirectIndYX },
  { "movw ", SNESSMP::DirectYa },
  { "mov  ", SNESSMP::DirectIndXY },

  { "dec  ", SNESSMP::Y },
  { "mov  ", SNESSMP::AY },
  { "cbne ", SNESSMP::DirectIndXRelative },
  { "daa  ", SNESSMP::A },

  //0xe0 - 0xef
  { "clrv ", SNESSMP::Implied },
  { "tcall", SNESSMP::TVector },
  { "set7 ", SNESSMP::Direct },
  { "bbs7 ", SNESSMP::DirectRelative },

  { "mov  ", SNESSMP::ADirect },
  { "mov  ", SNESSMP::AAbsolute },
  { "mov  ", SNESSMP::AIX },
  { "mov  ", SNESSMP::AIDirectIndX },

  { "mov  ", SNESSMP::AConstant },
  { "mov  ", SNESSMP::XAbsolute },
  { "not1 ", SNESSMP::CAbsoluteBit },
  { "mov  ", SNESSMP::YDirect },

  { "mov  ", SNESSMP::YAbsolute },
  { "notc ", SNESSMP::Implied },
  { "pop  ", SNESSMP::Y },
  { "sleep", SNESSMP::Implied },

  //0xf0 - 0xff
  { "beq  ", SNESSMP::Relative },
  { "tcall", SNESSMP::TVector },
  { "clr7 ", SNESSMP::Direct },
  { "bbc7 ", SNESSMP::DirectRelative },

  { "mov  ", SNESSMP::ADirectIndX },
  { "mov  ", SNESSMP::AAbsoluteX },
  { "mov  ", SNESSMP::AAbsoluteY },
  { "mov  ", SNESSMP::AIDirectIndY },

  { "mov  ", SNESSMP::XDirect },
  { "mov  ", SNESSMP::XDirectIndY },
  { "mov  ", SNESSMP::DirectDirect },
  { "mov  ", SNESSMP::YDirectIndX },

  { "inc  ", SNESSMP::Y },
  { "mov  ", SNESSMP::YA },
  { "dbz  ", SNESSMP::YRelative },
  { "stop ", SNESSMP::Implied },
};

inline unsigned SNESSMP::getOpcodeLength(uint8_t opcode) {
  switch(smpOpcodeInfo[opcode].mode) { default:
    case Implied:         return 1;  //
    case TVector:         return 1;  //0
    case Direct:          return 2;  //$00
    case DirectRelative:  return 3;  //$00,+/-$00
    case ADirect:         return 2;  //a,$00
    case AAbsolute:       return 3;  //a,$0000
    case AIX:             return 1;  //a,(x)
    case AIDirectIndX:    return 2;  //a,($00+x)
    case AConstant:       return 2;  //a,#$00
    case DirectDirect:    return 3;  //$00,$00
    case CAbsoluteBit:    return 3;  //c,$0000:0
    case Absolute:        return 3;  //$0000
    case P:               return 1;  //p
    case AbsoluteA:       return 3;  //$0000,a
    case Relative:        return 2;  //+/-$00
    case ADirectIndX:     return 2;  //a,$00+x
    case AAbsoluteX:      return 3;  //a,$0000+x
    case AAbsoluteY:      return 3;  //a,$0000+y
    case AIDirectIndY:    return 2;  //a,($00)+y
    case DirectConstant:  return 3;  //$00,#$00
    case IXIY:            return 1;  //(x),(y)
    case DirectIndX:      return 2;  //$00+x
    case A:               return 1;  //a
    case X:               return 1;  //x
    case XAbsolute:       return 3;  //x,$0000
    case IAbsoluteX:      return 3;  //($0000+x)
    case CNAbsoluteBit:   return 3;  //c,!$0000:0
    case XDirect:         return 2;  //x,$00
    case PVector:         return 2;  //$ff00
    case YaDirect:        return 2;  //ya,$00
    case XA:              return 1;  //x,a
    case YAbsolute:       return 3;  //y,$0000
    case Y:               return 1;  //y
    case AX:              return 1;  //a,x
    case YDirect:         return 2;  //y,$00
    case YConstant:       return 2;  //y,#$00
    case XSp:             return 1;  //x,sp
    case YaX:             return 1;  //ya,x
    case IXPA:            return 1;  //(x)+,a
    case SpX:             return 1;  //sp,x
    case AIXP:            return 1;  //a,(x)+
    case DirectA:         return 2;  //$00,a
    case IXA:             return 1;  //(x),a
    case IDirectIndXA:    return 2;  //($00+x),a
    case XConstant:       return 2;  //x,#$00
    case AbsoluteX:       return 3;  //$0000,x
    case AbsoluteBitC:    return 3;  //$0000:0,c
    case DirectY:         return 2;  //$00,y
    case AbsoluteY:       return 3;  //$0000,y
    case Ya:              return 1;  //ya
    case DirectIndXA:     return 2;  //$00+x,a
    case AbsoluteXA:      return 3;  //$0000+x,a
    case AbsoluteYA:      return 3;  //$0000+y,a
    case IDirectIndYA:    return 2;  //($00)+y,a
	case DirectX:         return 2; //$00,x
    case DirectIndYX:     return 2;  //$00+y,x
    case DirectYa:        return 2;  //$00,ya
    case DirectIndXY:     return 2;  //$00+x,y
    case AY:              return 1;  //a,y
    case DirectIndXRelative: return 3;  //$00+x,+/-$00
    case XDirectIndY:     return 2;  //x,$00+y
    case YDirectIndX:     return 2;  //y,$00+x
    case YA:              return 1;  //y,a
    case YRelative:       return 2;  //y,+/-$00
  }
}

inline bool SNESSMP::getOpcodeIndirect(uint8_t opcode) {
  switch(smpOpcodeInfo[opcode].mode) {
    case TVector:
    case AIX:
    case AIDirectIndX:
    case AIDirectIndY:
    case IXIY:
    case IAbsoluteX:
    case IXPA:
    case AIXP:
    case IXA:
    case IDirectIndXA:
    case IDirectIndYA:
      return true;

    default:
      return false;
  }
}

inline string SNESSMP::disassemble(uint16_t pc, uint8_t opcode, uint8_t pl, uint8_t ph, bool p) {
  string name = smpOpcodeInfo[opcode].name;
  unsigned mode = smpOpcodeInfo[opcode].mode;
  unsigned pdl = (p << 8) + pl;
  unsigned pdh = (p << 8) + ph;
  unsigned pa = (ph << 8) + pl;

  if(mode == Implied) return name;
  if(mode == TVector) return { name, " ", opcode >> 4 };
  if(mode == Direct) return { name, " $", hex<3>(pdl) };
  if(mode == DirectRelative) return { name, " $", hex<3>(pdl), ",$", hex<4>(pc + 3 + (int8_t)ph) };
  if(mode == ADirect) return { name, " a, $", hex<3>(pdl) };
  if(mode == AAbsolute) return { name, " a, $", hex<4>(pa) };
  if(mode == AIX) return { name, "a,(x)" };
  if(mode == AIDirectIndX) return { name, " a, ($", hex<3>(pdl), "+x)" };
  if(mode == AConstant) return { name, " a, #$", hex<2>(pl) };
  if(mode == DirectDirect) return { name, " $", hex<3>(pdh), ", $", hex<3>(pdl) };
  if(mode == CAbsoluteBit) return { name, " c, $", hex<4>(pa & 0x1fff), ":", pa >> 13 };
  if(mode == Absolute) return { name, " $", hex<4>(pa) };
  if(mode == P) return { name, " p" };
  if(mode == AbsoluteA) return { name, " $", hex<4>(pa), ",a" };
  if(mode == Relative) return { name, " $", hex<4>(pc + 2 + (int8_t)pl) };
  if(mode == ADirectIndX) return { name, " a, $", hex<3>(pdl), "+x" };
  if(mode == AAbsoluteX) return { name, " a, $", hex<4>(pa), "+x" };
  if(mode == AAbsoluteY) return { name, " a, $", hex<4>(pa), "+y" };
  if(mode == AIDirectIndY) return { name, " a, ($", hex<3>(pdl), ")+y" };
  if(mode == DirectConstant) return { name, " $", hex<3>(pdh), ", #$", hex<2>(pl) };
  if(mode == IXIY) return { name, " (x), (y)" };
  if(mode == DirectIndX) return { name, " $", hex<3>(pdl), "+x" };
  if(mode == A) return { name, " a" };
  if(mode == X) return { name, " x" };
  if(mode == XAbsolute) return { name, " x, $", hex<4>(pa) };
  if(mode == IAbsoluteX) return { name, " ($", hex<4>(pa), "+x)" };
  if(mode == CNAbsoluteBit) return { name, " c, !$", hex<4>(pa & 0x1fff), ":", pa >> 13 };
  if(mode == XDirect) return { name, " x, $", hex<3>(pdl) };
  if(mode == PVector) return { name, " $ff", hex<2>(pl) };
  if(mode == YaDirect) return { name, " ya, $", hex<3>(pdl) };
  if(mode == XA) return { name, " x, a" };
  if(mode == YAbsolute) return { name, " y, $", hex<4>(pa) };
  if(mode == Y) return { name, " y" };
  if(mode == AX) return { name, " a, x" };
  if(mode == YDirect) return { name, " y, $", hex<3>(pdl) };
  if(mode == YConstant) return { name, " y, #$", hex<2>(pl) };
  if(mode == XSp) return { name, " x, sp" };
  if(mode == YaX) return { name, " ya, x" };
  if(mode == IXPA) return { name, " (x)+, a" };
  if(mode == SpX) return { name, " sp, x" };
  if(mode == AIXP) return { name, " a, (x)+" };
  if(mode == DirectA) return { name, " $", hex<3>(pdl), ", a" };
  if(mode == IXA) return { name, " (x), a" };
  if(mode == IDirectIndXA) return { name, " ($", hex<3>(pdl), "+x), a" };
  if(mode == XConstant) return { name, " x, #$", hex<2>(pl) };
  if(mode == AbsoluteX) return { name, " $", hex<4>(pa), ", x" };
  if(mode == AbsoluteBitC) return { name, " $", hex<4>(pa & 0x1fff), ":", pa >> 13, ", c" };
  if(mode == DirectY) return { name, " $", hex<3>(pdl), ", y" };
  if(mode == AbsoluteY) return { name, " $", hex<4>(pa), ", y" };
  if(mode == Ya) return { name, " ya" };
  if(mode == DirectIndXA) return { name, " $", hex<3>(pdl), "+x, a" };
  if(mode == AbsoluteXA) return { name, " $", hex<4>(pa), "+x, a" };
  if(mode == AbsoluteYA) return { name, " $", hex<4>(pa), "+y, a" };
  if(mode == IDirectIndYA) return { name, " ($", hex<3>(pdl), ")+y, a" };
  if(mode == DirectX) return { name, " $", hex<3>(pdl), ", x" };
  if(mode == DirectIndYX) return { name, " $", hex<3>(pdl), "+y, x" };
  if(mode == DirectYa) return { name, " $", hex<3>(pdl), ",ya" };
  if(mode == DirectIndXY) return { name, " $", hex<3>(pdl), "+x, y" };
  if(mode == AY) return { name, " a, y" };
  if(mode == DirectIndXRelative) return { name, " $", hex<3>(pdl), ", $", hex<4>(pc + 3 + (int8_t)ph) };
  if(mode == XDirectIndY) return { name, " x, $", hex<3>(pdl), "+y" };
  if(mode == YDirectIndX) return { name, " y, $", hex<3>(pdl), "+x" };
  if(mode == YA) return { name, " y, a" };
  if(mode == YRelative) return { name, " y, $", hex<4>(pc + 2 + (int8_t)pl) };

  return "";
}

}

#endif
