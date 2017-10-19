enum {
  OPTYPE_DP = 0,    //dp
  OPTYPE_DPX,       //dp,x
  OPTYPE_DPY,       //dp,y
  OPTYPE_IDP,       //(dp)
  OPTYPE_IDPX,      //(dp,x)
  OPTYPE_IDPY,      //(dp),y
  OPTYPE_ILDP,      //[dp]
  OPTYPE_ILDPY,     //[dp],y
  OPTYPE_ADDR,      //addr
  OPTYPE_ADDRX,     //addr,x
  OPTYPE_ADDRY,     //addr,y
  OPTYPE_IADDRX,    //(addr,x)
  OPTYPE_ILADDR,    //[addr]
  OPTYPE_LONG,      //long
  OPTYPE_LONGX,     //long, x
  OPTYPE_SR,        //sr,s
  OPTYPE_ISRY,      //(sr,s),y
  OPTYPE_ADDR_PC,   //pbr:addr
  OPTYPE_IADDR_PC,  //pbr:(addr)
  OPTYPE_RELB,      //relb
  OPTYPE_RELW,      //relw
  OPTYPE_IMPL,      //-
  OPTYPE_IM,        //#123
  OPTYPE_A,         //A
  OPTYPE_MV,        //bnk,bnk
};

struct Opcode {
  enum Flag {
    FLAG_BRA = 0x01,
    FLAG_BRA_CONTINUE = 0x02,
    FLAG_RESET_M = 0x10,
    FLAG_RESET_X = 0x20,
    FLAG_SET_M = 0x40,
    FLAG_SET_X = 0x80,
    FLAG_PUSH_P = 0x100,
    FLAG_POP_P = 0x200,
    FLAG_RETURN = 0x400,
    FLAG_BRK = 0x1000,
    FLAG_RESET_E = 0x8000
  };

  void set(uint16 flags, uint8 optype, const char *opcode, uint8 (&param)[4], uint8 paramsize=8) {
    this->flags = flags;
    this->optype = optype;
    this->opcode = opcode;

    *((uint32*)&this->param) = *((uint32*)&param);
    this->paramsize = paramsize;
  }

  uint8 size() const {
    switch (optype) {
      case OPTYPE_IMPL:
      case OPTYPE_A:
        return 1;

      case OPTYPE_DP:
      case OPTYPE_DPX:
      case OPTYPE_DPY:
      case OPTYPE_IDP:
      case OPTYPE_IDPX:
      case OPTYPE_IDPY:
      case OPTYPE_ILDP:
      case OPTYPE_ILDPY:
      case OPTYPE_SR:
      case OPTYPE_ISRY:
      case OPTYPE_RELB:
        return 2;

      case OPTYPE_ADDR:
      case OPTYPE_ADDRX:
      case OPTYPE_ADDRY:
      case OPTYPE_IADDRX:
      case OPTYPE_ILADDR:
      case OPTYPE_ADDR_PC:
      case OPTYPE_IADDR_PC:
      case OPTYPE_RELW:
      case OPTYPE_MV:
        return 3;

      case OPTYPE_LONG:
      case OPTYPE_LONGX:
        return 4;

      case OPTYPE_IM:
        return (paramsize == 8 ? 2 : 3);
    }
  }

  bool isIndirect() const {
    switch (optype) {
    case OPTYPE_IDP:
    case OPTYPE_IDPX:
    case OPTYPE_IDPY:
    case OPTYPE_ILDP:
    case OPTYPE_ILDPY:
    case OPTYPE_IADDRX:
    case OPTYPE_ILADDR:
      return true;

    default:
      return false;
    }
  }

  inline bool isBra() const { return flags & FLAG_BRA; }
  inline bool isBraWithContinue() const { return flags & FLAG_BRA_CONTINUE; }
  inline bool resetsX() const { return flags & FLAG_RESET_X; }
  inline bool resetsM() const { return flags & FLAG_RESET_M; }
  inline bool resetsE() const { return flags & FLAG_RESET_E; }
  inline bool setsX() const { return flags & FLAG_SET_X; }
  inline bool setsM() const { return flags & FLAG_SET_M; }
  inline bool pushesP() const { return flags & FLAG_PUSH_P; }
  inline bool popsP() const { return flags & FLAG_POP_P; }
  inline bool breaks() const { return flags & FLAG_BRK; }
  inline bool returns() const { return flags & FLAG_RETURN; }

  uint8 op8() {
    return param[1];
  }

  uint16 op16() {
    return *((uint16*)&param[1]);
  }

  uint32 op24() {
    return *((uint16*)&param[1]) | param[3] << 16;
  }

  uint32 opall() {
    switch (size()) {
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
