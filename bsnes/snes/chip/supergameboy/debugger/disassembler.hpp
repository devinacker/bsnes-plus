struct Opcode {
  enum Flag {
    FLAG_BRA = 0x01, // jumps and unconditional branches
    FLAG_BRA_CONTINUE = 0x02, // calls and conditional branches
    FLAG_INDIRECT = 0x04, // indirect memory accesses
    FLAG_PUSH_F = 0x10, // pushes flags
    FLAG_POP_F = 0x20, // pops flags
    FLAG_RETURN = 0x40, // returns from call (unconditionally)
  };

  void set(uint16 flags, uint8 optype1, uint8 optype2, const char *opcode, uint8 (&param)[3], uint8 paramsize=0) {
    this->flags = flags;
    this->optype[0] = optype1;
    this->optype[1] = optype2;
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
  inline bool pushesF() const { return flags & FLAG_PUSH_F; }
  inline bool popsF() const { return flags & FLAG_POP_F; }
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
  uint8 optype[2];
  uint8 param[3];
  uint8 paramsize;
  const char *opcode;
};

void disassemble_opcode(char *output, uint24 addr);
void disassemble_opcode_ex(Opcode &opcode, uint24 addr);
inline uint8 dreadb(uint16 addr);
inline uint16 dreadw(uint16 addr);
inline uint16 relb(int8 offset, int op_len, uint16 pc);
uint24 decode(uint8 offset_type, uint16 addr, uint16 pc);
