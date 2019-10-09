struct Opcode {
  enum Flag {
    FLAG_BRA = 0x01, // jumps and unconditional branches
    FLAG_BRA_CONTINUE = 0x02, // conditional branches
    FLAG_INDIRECT = 0x04, // indirect memory accesses
    FLAG_SET_ALT1 = 0x10, // sets ALT1 flag
    FLAG_SET_ALT2 = 0x20, // sets ALT2 flag
    FLAG_RETURN = 0x100, // returns from call
  };

  void set(uint16 flags, uint8 optype, const char *opcode, uint8 (&param)[3], uint8 paramsize=0) {
    this->flags = flags;
    this->optype = optype;
    this->opcode = opcode;

    this->param[0] = param[0];
    this->param[1] = param[1];
    this->param[2] = param[2];
    this->paramsize = paramsize;
  }

  uint8 size() const {
    return 1 + paramsize;
  }

  inline bool isBra() const { return flags & FLAG_BRA; }
  inline bool isBraWithContinue() const { return flags & FLAG_BRA_CONTINUE; }
  inline bool isIndirect() const { return flags & FLAG_INDIRECT; }
  inline bool setsAlt1() const { return flags & FLAG_SET_ALT1; }
  inline bool setsAlt2() const { return flags & FLAG_SET_ALT2; }
  inline bool returns() const { return flags & FLAG_RETURN; }

  uint8 op4() {
    return param[0] & 15;
  }

  uint8 op8(unsigned index = 0) {
    return param[1 + index];
  }

  uint16 op16() {
    return param[1] | param[2] << 8;
  }

  uint32 opall() {
    switch (size()) {
      default: return 0;
      case 1:  return op4();
      case 2:  return op8();
      case 3:  return op16();
    }
  }

  uint16 flags;
  uint8 optype;
  uint8 param[3];
  uint8 paramsize;
  const char *opcode;
};

enum : unsigned {
  Implied,        //
  ImpliedPlot,    //r1,r2
  ImpliedLongMul, //r4,r6
  ImpliedFixedMul,//r6
  ImpliedMerge,   //r7,r8
  ImpliedLoop,    //r12,r13
  ImpliedROM,     //
  ImpliedRAM,     //
  Relative,       //+/- $nn
  LinkRelative,   //+ $n (1-4)
  Register,       //Rn
  RegisterJump,   //Rn
  RegisterLJump,  //Rn
  IRegister,      //(Rn)
  RegImmediate,   //Rn,#$nn
  RegImmediateLong,//Rn,#$nnnn
  RegDirect,      //Rn,$nnn (ram buffer)
  RegAbsolute,    //Rn,$nnnn (ram buffer)
  ImmediateShort, //#n (0-15)
};

struct OpcodeInfo {
  const char *name;
  unsigned mode;
};

OpcodeInfo opcode_info(bool alt1, bool alt2, uint8 opcode) const;

void disassemble_opcode(char *output, uint32 addr, bool track_regs = false);
void disassemble_opcode_ex(Opcode &opcode, uint32 addr, bool alt1, bool alt2);
uint8 opcode_length(uint8 offset_type);
uint32 decode(uint8 offset_type, uint32 addr, uint32 pc);

uint16 disassemble_lastregs;