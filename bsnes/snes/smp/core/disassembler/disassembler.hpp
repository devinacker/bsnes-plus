struct Opcode {
  enum Flag {
    FLAG_BRA = 0x01, // jumps and unconditional branches
    FLAG_BRA_CONTINUE = 0x02, // calls and conditional branches
    FLAG_INDIRECT = 0x04, // indirect memory accesses
    FLAG_RESET_P = 0x10, // clears P flag
    FLAG_SET_P = 0x20, // sets P flag
    FLAG_PUSH_P = 0x100, // pushes flags
    FLAG_POP_P = 0x200, // pops flags
    FLAG_RETURN = 0x400, // returns from call
    FLAG_BRK = 0x1000, // software interrupt
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
  inline bool resetsP() const { return flags & FLAG_RESET_P; }
  inline bool setsP() const { return flags & FLAG_SET_P; }
  inline bool pushesP() const { return flags & FLAG_PUSH_P; }
  inline bool popsP() const { return flags & FLAG_POP_P; }
  inline bool breaks() const { return flags & FLAG_BRK; }
  inline bool returns() const { return flags & FLAG_RETURN; }

  uint8 op() {
    return param[0];
  }

  uint8 op8(unsigned index = 0) {
    return param[1 + index];
  }

  uint16 op16() {
    return param[1] | param[2] << 8;
  }

  uint32 opall() {
    switch (size()) {
      default:
      case 1: return 0;
      case 2: return op8();
      case 3: return op16();
    }
  }

  uint16 flags;
  uint8 optype;
  uint8 param[3];
  uint8 paramsize;
  const char *opcode;
};

void disassemble_opcode(char *output, uint16 addr);
void disassemble_opcode_ex(Opcode &opcode, uint16 addr);
inline uint8 dreadb(uint16 addr);
inline uint16 dreadw(uint16 addr);
inline uint16 relb(int8 offset, int op_len, uint16 pc);
uint16 decode(uint8 offset_type, uint16 addr, uint16 pc);
