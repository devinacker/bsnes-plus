#ifndef NALL_SNES_CPU_HPP
#define NALL_SNES_CPU_HPP

namespace nall {

struct SNESCPU {
  enum : unsigned {
    Implied,        //
    Constant,       //#$00
    AccumConstant,  //#$00
    IndexConstant,  //#$00
    Direct,         //$00
    DirectX,        //$00,x
    DirectY,        //$00,y
    IDirect,        //($00)
    IDirectX,       //($00,x)
    IDirectY,       //($00),y
    ILDirect,       //[$00]
    ILDirectY,      //[$00],y
    Address,        //$0000
    AddressX,       //$0000,x
    AddressY,       //$0000,y
    IAddressX,      //($0000,x)
    ILAddress,      //[$0000]
    PAddress,       //PBR:$0000
    PIAddress,      //PBR:($0000)
    Long,           //$000000
    LongX,          //$000000,x
    Stack,          //$00,s
    IStackY,        //($00,s),y
    BlockMove,      //$00,$00
    RelativeShort,  //+/- $00
    RelativeLong,   //+/- $0000
  };

  struct OpcodeInfo {
    char name[4];
    unsigned mode;
  };
  
  static unsigned getOpcodeLength(bool accum, bool index, uint8_t opcode);
  static bool getOpcodeIndirect(uint8_t opcode);
  static string disassemble(unsigned pc, bool accum, bool index, uint8_t opcode, uint8_t pl, uint8_t ph, uint8_t pb);
};

static const SNESCPU::OpcodeInfo cpuOpcodeInfo[256] = {
  //0x00 - 0x0f
  { "brk", SNESCPU::Constant },
  { "ora", SNESCPU::IDirectX },
  { "cop", SNESCPU::Constant },
  { "ora", SNESCPU::Stack },
  
  { "tsb", SNESCPU::Direct },
  { "ora", SNESCPU::Direct },
  { "asl", SNESCPU::Direct },
  { "ora", SNESCPU::ILDirect },
  
  { "php", SNESCPU::Implied },
  { "ora", SNESCPU::AccumConstant },
  { "asl", SNESCPU::Implied },
  { "phd", SNESCPU::Implied },
  
  { "tsb", SNESCPU::Address },
  { "ora", SNESCPU::Address },
  { "asl", SNESCPU::Address },
  { "ora", SNESCPU::Long },
  
  //0x10 - 0x1f
  { "bpl", SNESCPU::RelativeShort },
  { "ora", SNESCPU::IDirectY },
  { "ora", SNESCPU::IDirect },
  { "ora", SNESCPU::IStackY },
  
  { "trb", SNESCPU::Direct },
  { "ora", SNESCPU::DirectX },
  { "asl", SNESCPU::DirectX },
  { "ora", SNESCPU::ILDirectY },
  
  { "clc", SNESCPU::Implied },
  { "ora", SNESCPU::AddressY },
  { "inc", SNESCPU::Implied },
  { "tcs", SNESCPU::Implied },
  
  { "trb", SNESCPU::Address },
  { "ora", SNESCPU::AddressX },
  { "asl", SNESCPU::AddressX },
  { "ora", SNESCPU::LongX },
  
  //0x20 - 0x2f
  { "jsr", SNESCPU::PAddress },
  { "and", SNESCPU::IDirectX },
  { "jsl", SNESCPU::Long },
  { "and", SNESCPU::Stack },
  
  { "bit", SNESCPU::Direct },
  { "and", SNESCPU::Direct },
  { "rol", SNESCPU::Direct },
  { "and", SNESCPU::ILDirect },
  
  { "plp", SNESCPU::Implied },
  { "and", SNESCPU::AccumConstant },
  { "rol", SNESCPU::Implied },
  { "pld", SNESCPU::Implied },
  
  { "bit", SNESCPU::Address },
  { "and", SNESCPU::Address },
  { "rol", SNESCPU::Address },
  { "and", SNESCPU::Long },
  
  //0x30 - 0x3f
  { "bmi", SNESCPU::RelativeShort },
  { "and", SNESCPU::IDirectY },
  { "and", SNESCPU::IDirect },
  { "and", SNESCPU::IStackY },
  
  { "bit", SNESCPU::DirectX },
  { "and", SNESCPU::DirectX },
  { "rol", SNESCPU::DirectX },
  { "and", SNESCPU::ILDirectY },
  
  { "sec", SNESCPU::Implied },
  { "and", SNESCPU::AddressY },
  { "dec", SNESCPU::Implied },
  { "tsc", SNESCPU::Implied },
  
  { "bit", SNESCPU::AddressX },
  { "and", SNESCPU::AddressX },
  { "rol", SNESCPU::AddressX },
  { "and", SNESCPU::LongX },
  
  //0x40 - 0x4f
  { "rti", SNESCPU::Implied },
  { "eor", SNESCPU::IDirectX },
  { "wdm", SNESCPU::Constant },
  { "eor", SNESCPU::Stack },
  
  { "mvp", SNESCPU::BlockMove },
  { "eor", SNESCPU::Direct },
  { "lsr", SNESCPU::Direct },
  { "eor", SNESCPU::ILDirect },
  
  { "pha", SNESCPU::Implied },
  { "eor", SNESCPU::AccumConstant },
  { "lsr", SNESCPU::Implied },
  { "phk", SNESCPU::Implied },
  
  { "jmp", SNESCPU::PAddress },
  { "eor", SNESCPU::Address },
  { "lsr", SNESCPU::Address },
  { "eor", SNESCPU::Long },
  
  //0x50 - 0x5f
  { "bvc", SNESCPU::RelativeShort },
  { "eor", SNESCPU::IDirectY },
  { "eor", SNESCPU::IDirect },
  { "eor", SNESCPU::IStackY },
  
  { "mvn", SNESCPU::BlockMove },
  { "eor", SNESCPU::DirectX },
  { "lsr", SNESCPU::DirectX },
  { "eor", SNESCPU::ILDirectY },
  
  { "cli", SNESCPU::Implied },
  { "eor", SNESCPU::AddressY },
  { "phy", SNESCPU::Implied },
  { "tcd", SNESCPU::Implied },
  
  { "jml", SNESCPU::Long },
  { "eor", SNESCPU::AddressX },
  { "lsr", SNESCPU::AddressX },
  { "eor", SNESCPU::LongX },
  
  //0x60 - 0x6f
  { "rts", SNESCPU::Implied },
  { "adc", SNESCPU::IDirectX },
  { "per", SNESCPU::Address },
  { "adc", SNESCPU::Stack },
  
  { "stz", SNESCPU::Direct },
  { "adc", SNESCPU::Direct },
  { "ror", SNESCPU::Direct },
  { "adc", SNESCPU::ILDirect },
  
  { "pla", SNESCPU::Implied },
  { "adc", SNESCPU::AccumConstant },
  { "ror", SNESCPU::Implied },
  { "rtl", SNESCPU::Implied },
  
  { "jmp", SNESCPU::PIAddress },
  { "adc", SNESCPU::Address },
  { "ror", SNESCPU::Address },
  { "adc", SNESCPU::Long },
  
  //0x70 - 0x7f
  { "bvs", SNESCPU::RelativeShort },
  { "adc", SNESCPU::IDirectY },
  { "adc", SNESCPU::IDirect },
  { "adc", SNESCPU::IStackY },
  
  { "stz", SNESCPU::DirectX },
  { "adc", SNESCPU::DirectX },
  { "ror", SNESCPU::DirectX },
  { "adc", SNESCPU::ILDirectY },
  
  { "sei", SNESCPU::Implied },
  { "adc", SNESCPU::AddressY },
  { "ply", SNESCPU::Implied },
  { "tdc", SNESCPU::Implied },
  
  { "jmp", SNESCPU::IAddressX },
  { "adc", SNESCPU::AddressX },
  { "ror", SNESCPU::AddressX },
  { "adc", SNESCPU::LongX },
  
  //0x80 - 0x8f
  { "bra", SNESCPU::RelativeShort },
  { "sta", SNESCPU::IDirectX },
  { "brl", SNESCPU::RelativeLong },
  { "sta", SNESCPU::Stack },
  
  { "sty", SNESCPU::Direct },
  { "sta", SNESCPU::Direct },
  { "stx", SNESCPU::Direct },
  { "sta", SNESCPU::ILDirect },
  
  { "dey", SNESCPU::Implied },
  { "bit", SNESCPU::AccumConstant },
  { "txa", SNESCPU::Implied },
  { "phb", SNESCPU::Implied },
  
  { "sty", SNESCPU::Address },
  { "sta", SNESCPU::Address },
  { "stx", SNESCPU::Address },
  { "sta", SNESCPU::Long },
  
  //0x90 - 0x9f
  { "bcc", SNESCPU::RelativeShort },
  { "sta", SNESCPU::IDirectY },
  { "sta", SNESCPU::IDirect },
  { "sta", SNESCPU::IStackY },
  
  { "sty", SNESCPU::DirectX },
  { "sta", SNESCPU::DirectX },
  { "stx", SNESCPU::DirectY },
  { "sta", SNESCPU::ILDirectY },
  
  { "tya", SNESCPU::Implied },
  { "sta", SNESCPU::AddressY },
  { "txs", SNESCPU::Implied },
  { "txy", SNESCPU::Implied },
  
  { "stz", SNESCPU::Address },
  { "sta", SNESCPU::AddressX },
  { "stz", SNESCPU::AddressX },
  { "sta", SNESCPU::LongX },
  
  //0xa0 - 0xaf
  { "ldy", SNESCPU::IndexConstant },
  { "lda", SNESCPU::IDirectX },
  { "ldx", SNESCPU::IndexConstant },
  { "lda", SNESCPU::Stack },
  
  { "ldy", SNESCPU::Direct },
  { "lda", SNESCPU::Direct },
  { "ldx", SNESCPU::Direct },
  { "lda", SNESCPU::ILDirect },
  
  { "tay", SNESCPU::Implied },
  { "lda", SNESCPU::AccumConstant },
  { "tax", SNESCPU::Implied },
  { "plb", SNESCPU::Implied },
  
  { "ldy", SNESCPU::Address },
  { "lda", SNESCPU::Address },
  { "ldx", SNESCPU::Address },
  { "lda", SNESCPU::Long },
  
  //0xb0 - 0xbf
  { "bcs", SNESCPU::RelativeShort },
  { "lda", SNESCPU::IDirectY },
  { "lda", SNESCPU::IDirect },
  { "lda", SNESCPU::IStackY },
  
  { "ldy", SNESCPU::DirectX },
  { "lda", SNESCPU::DirectX },
  { "ldx", SNESCPU::DirectY },
  { "lda", SNESCPU::ILDirectY },
  
  { "clv", SNESCPU::Implied },
  { "lda", SNESCPU::AddressY },
  { "tsx", SNESCPU::Implied },
  { "tyx", SNESCPU::Implied },
  
  { "ldy", SNESCPU::AddressX },
  { "lda", SNESCPU::AddressX },
  { "ldx", SNESCPU::AddressY },
  { "lda", SNESCPU::LongX },
  
  //0xc0 - 0xcf
  { "cpy", SNESCPU::IndexConstant },
  { "cmp", SNESCPU::IDirectX },
  { "rep", SNESCPU::Constant },
  { "cmp", SNESCPU::Stack },
  
  { "cpy", SNESCPU::Direct },
  { "cmp", SNESCPU::Direct },
  { "dec", SNESCPU::Direct },
  { "cmp", SNESCPU::ILDirect },
  
  { "iny", SNESCPU::Implied },
  { "cmp", SNESCPU::AccumConstant },
  { "dex", SNESCPU::Implied },
  { "wai", SNESCPU::Implied },
  
  { "cpy", SNESCPU::Address },
  { "cmp", SNESCPU::Address },
  { "dec", SNESCPU::Address },
  { "cmp", SNESCPU::Long },
  
  //0xd0 - 0xdf
  { "bne", SNESCPU::RelativeShort },
  { "cmp", SNESCPU::IDirectY },
  { "cmp", SNESCPU::IDirect },
  { "cmp", SNESCPU::IStackY },
  
  { "pei", SNESCPU::IDirect },
  { "cmp", SNESCPU::DirectX },
  { "dec", SNESCPU::DirectX },
  { "cmp", SNESCPU::ILDirectY },
  
  { "cld", SNESCPU::Implied },
  { "cmp", SNESCPU::AddressY },
  { "phx", SNESCPU::Implied },
  { "stp", SNESCPU::Implied },
  
  { "jmp", SNESCPU::ILAddress },
  { "cmp", SNESCPU::AddressX },
  { "dec", SNESCPU::AddressX },
  { "cmp", SNESCPU::LongX },
  
  //0xe0 - 0xef
  { "cpx", SNESCPU::IndexConstant },
  { "sbc", SNESCPU::IDirectX },
  { "sep", SNESCPU::Constant },
  { "sbc", SNESCPU::Stack },
  
  { "cpx", SNESCPU::Direct },
  { "sbc", SNESCPU::Direct },
  { "inc", SNESCPU::Direct },
  { "sbc", SNESCPU::ILDirect },
  
  { "inx", SNESCPU::Implied },
  { "sbc", SNESCPU::AccumConstant },
  { "nop", SNESCPU::Implied },
  { "xba", SNESCPU::Implied },
  
  { "cpx", SNESCPU::Address },
  { "sbc", SNESCPU::Address },
  { "inc", SNESCPU::Address },
  { "sbc", SNESCPU::Long },
  
  //0xf0 - 0xff
  { "beq", SNESCPU::RelativeShort },
  { "sbc", SNESCPU::IDirectY },
  { "sbc", SNESCPU::IDirect },
  { "sbc", SNESCPU::IStackY },
  
  { "pea", SNESCPU::Address },
  { "sbc", SNESCPU::DirectX },
  { "inc", SNESCPU::DirectX },
  { "sbc", SNESCPU::ILDirectY },
  
  { "sed", SNESCPU::Implied },
  { "sbc", SNESCPU::AddressY },
  { "plx", SNESCPU::Implied },
  { "xce", SNESCPU::Implied },
  
  { "jsr", SNESCPU::IAddressX },
  { "sbc", SNESCPU::AddressX },
  { "inc", SNESCPU::AddressX },
  { "sbc", SNESCPU::LongX },
};

inline unsigned SNESCPU::getOpcodeLength(bool accum, bool index, uint8_t opcode) {
  switch(cpuOpcodeInfo[opcode].mode) { default:
    case Implied: return 1;
    case Constant: return 2;
    case AccumConstant: return 3 - accum;
    case IndexConstant: return 3 - index;
    case Direct: return 2;
    case DirectX: return 2;
    case DirectY: return 2;
    case IDirect: return 2;
    case IDirectX: return 2;
    case IDirectY: return 2;
    case ILDirect: return 2;
    case ILDirectY: return 2;
    case Address: return 3;
    case AddressX: return 3;
    case AddressY: return 3;
    case IAddressX: return 3;
    case ILAddress: return 3;
    case PAddress: return 3;
    case PIAddress: return 3;
    case Long: return 4;
    case LongX: return 4;
    case Stack: return 2;
    case IStackY: return 2;
    case BlockMove: return 3;
    case RelativeShort: return 2;
    case RelativeLong: return 3;
  }
}

inline bool SNESCPU::getOpcodeIndirect(uint8_t opcode) {
  switch(cpuOpcodeInfo[opcode].mode) {
    case IDirect:
    case IDirectX:
    case IDirectY:
    case ILDirect:
    case ILDirectY:
    case IAddressX:
    case ILAddress:
      return true;

    default:
      return false;
  }
}

inline string SNESCPU::disassemble(unsigned pc, bool accum, bool index, uint8_t opcode, uint8_t pl, uint8_t ph, uint8_t pb) {
  string name = cpuOpcodeInfo[opcode].name;
  unsigned mode = cpuOpcodeInfo[opcode].mode;

  if(mode == Implied) return name;
  if(mode == Constant) return { name, " #$", hex<2>(pl) };
  if(mode == AccumConstant) return { name, " #$", accum ? "" : hex<2>(ph), hex<2>(pl) };
  if(mode == IndexConstant) return { name, " #$", index ? "" : hex<2>(ph), hex<2>(pl) };
  if(mode == Direct) return { name, " $", hex<2>(pl) };
  if(mode == DirectX) return { name, " $", hex<2>(pl), ",x" };
  if(mode == DirectY) return { name, " $", hex<2>(pl), ",y" };
  if(mode == IDirect) return { name, " ($", hex<2>(pl), ")" };
  if(mode == IDirectX) return { name, " ($", hex<2>(pl), ",x)" };
  if(mode == IDirectY) return { name, " ($", hex<2>(pl), "),y" };
  if(mode == ILDirect) return { name, " [$", hex<2>(pl), "]" };
  if(mode == ILDirectY) return { name, " [$", hex<2>(pl), "],y" };
  if(mode == Address) return { name, " $", hex<2>(ph), hex<2>(pl) };
  if(mode == AddressX) return { name, " $", hex<2>(ph), hex<2>(pl), ",x" };
  if(mode == AddressY) return { name, " $", hex<2>(ph), hex<2>(pl), ",y" };
  if(mode == IAddressX) return { name, " ($", hex<2>(ph), hex<2>(pl), ",x)" };
  if(mode == ILAddress) return { name, " [$", hex<2>(ph), hex<2>(pl), "]" };
  if(mode == PAddress) return { name, " $", hex<2>(ph), hex<2>(pl) };
  if(mode == PIAddress) return { name, " ($", hex<2>(ph), hex<2>(pl), ")" };
  if(mode == Long) return { name, " $", hex<2>(pb), hex<2>(ph), hex<2>(pl) };
  if(mode == LongX) return { name, " $", hex<2>(pb), hex<2>(ph), hex<2>(pl), ",x" };
  if(mode == Stack) return { name, " $", hex<2>(pl), ",s" };
  if(mode == IStackY) return { name, " ($", hex<2>(pl), ",s),y" };
  if(mode == BlockMove) return { name, " $", hex<2>(ph), ", $", hex<2>(pl) };
  if(mode == RelativeShort) {
    unsigned addr = (uint16_t)(pc + 2) + (int8_t)(pl << 0);
    return { name, " $", hex<4>(addr) };
  }
  if(mode == RelativeLong) {
    unsigned addr = (uint16_t)(pc + 3) + (int16_t)((ph << 8) + (pl << 0));
    return { name, " $", hex<4>(addr) };
  }

  return "";
}

}

#endif
