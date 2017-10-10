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
  void set(uint8 size, uint8 optype, const char *opcode, uint8 (&param)[4], uint8 paramsize=8) {
    this->size = size;
    this->optype = optype;
    this->opcode = opcode;

    *((uint32*)&this->param) = *((uint32*)&param);
    this->paramsize = paramsize;
  }

  uint8 op8() {
    return param[1];
  }

  uint16 op16() {
    return *((uint16*)&param[1]);
  }

  uint32 op24() {
    return *((uint16*)&param[1]) | param[3] << 16;
  }

  uint8 size;
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
