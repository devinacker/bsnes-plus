struct Opcode {
  enum Flag {
    FLAG_BRA = 0x01, // jumps and unconditional branches
    FLAG_BRA_CONTINUE = 0x02, // calls and conditional branches
    FLAG_INDIRECT = 0x04, // indirect memory accesses
    FLAG_RESET_M = 0x10, // clears M flag
    FLAG_RESET_X = 0x20, // clears X flag
    FLAG_SET_M = 0x40, // sets M flag
    FLAG_SET_X = 0x80, // sets X flag
    FLAG_PUSH_P = 0x100, // pushes flags
    FLAG_POP_P = 0x200, // pops flags
    FLAG_RETURN = 0x400, // returns from call
    FLAG_BRK = 0x1000, // software interrupt
    FLAG_RESET_E = 0x8000 // modifies E flag
  };

  void set(uint16 flags, uint8 optype, const char *opcode, uint8 (&param)[4], uint8 paramsize=0) {
    this->flags = flags;
    this->optype = optype;
    this->opcode = opcode;

    *((uint32*)&this->param) = *((uint32*)&param);
    this->paramsize = paramsize;
  }

  uint8 size() const {
    return 1 + paramsize;
  }

  inline bool isBra() const { return flags & FLAG_BRA; }
  inline bool isBraWithContinue() const { return flags & FLAG_BRA_CONTINUE; }
  inline bool isIndirect() const { return flags & FLAG_INDIRECT; }
  inline bool resetsX() const { return flags & FLAG_RESET_X; }
  inline bool resetsM() const { return flags & FLAG_RESET_M; }
  inline bool resetsE() const { return flags & FLAG_RESET_E; }
  inline bool setsX() const { return flags & FLAG_SET_X; }
  inline bool setsM() const { return flags & FLAG_SET_M; }
  inline bool pushesP() const { return flags & FLAG_PUSH_P; }
  inline bool popsP() const { return flags & FLAG_POP_P; }
  inline bool breaks() const { return flags & FLAG_BRK; }
  inline bool returns() const { return flags & FLAG_RETURN; }

  uint8 op8(unsigned index = 0) {
    return param[1 + index];
  }

  uint16 op16() {
    return param[1] | param[2] << 8;
  }

  uint32 op24() {
    return param[1] | param[2] << 8 | param[3] << 16;
  }

  uint32 opall() {
    switch (size()) {
      default:
      case 1: return 0;
      case 2: return op8();
      case 3: return op16();
      case 4: return op24();
    }
  }

  uint16 flags;
  uint8 optype;
  uint8 param[4];
  uint8 paramsize;
  const char *opcode;
};

void   disassemble_opcode(char *output, uint32 addr, bool hclocks = false);
void   disassemble_opcode_ex(Opcode &opcode, uint32 addr, bool e, bool m, bool x);
uint8  dreadb(uint32 addr);
uint16 dreadw(uint32 addr);
uint32 dreadl(uint32 addr);
uint32 decode(uint8 offset_type, uint32 addr, uint32 pc);
