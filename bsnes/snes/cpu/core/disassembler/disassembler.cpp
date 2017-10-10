#ifdef CPUCORE_CPP

uint8 CPUcore::disassembler_read(uint32 addr)
{
  return 0;
}

uint8 CPUcore::dreadb(uint32 addr) {
  return disassembler_read(addr);
}

uint16 CPUcore::dreadw(uint32 addr) {
  uint16 r;
  r  = dreadb(addr + 0) <<  0;
  r |= dreadb(addr + 1) <<  8;
  return r;
}

uint32 CPUcore::dreadl(uint32 addr) {
  uint32 r;
  r  = dreadb(addr + 0) <<  0;
  r |= dreadb(addr + 1) <<  8;
  r |= dreadb(addr + 2) << 16;
  return r;
}

uint32 CPUcore::decode(uint8 offset_type, uint32 addr, uint32 pc) {
  uint32 r = 0;

  switch(offset_type) {
    case OPTYPE_DP:
      r = (regs.d + (addr & 0xffff)) & 0xffff;
      break;
    case OPTYPE_DPX:
      r = (regs.d + regs.x + (addr & 0xffff)) & 0xffff;
      break;
    case OPTYPE_DPY:
      r = (regs.d + regs.y + (addr & 0xffff)) & 0xffff;
      break;
    case OPTYPE_IDP:
      addr = (regs.d + (addr & 0xffff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr);
      break;
    case OPTYPE_IDPX:
      addr = (regs.d + regs.x + (addr & 0xffff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr);
      break;
    case OPTYPE_IDPY:
      addr = (regs.d + (addr & 0xffff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr) + regs.y;
      break;
    case OPTYPE_ILDP:
      addr = (regs.d + (addr & 0xffff)) & 0xffff;
      r = dreadl(addr);
      break;
    case OPTYPE_ILDPY:
      addr = (regs.d + (addr & 0xffff)) & 0xffff;
      r = dreadl(addr) + regs.y;
      break;
    case OPTYPE_ADDR:
      r = (regs.db << 16) + (addr & 0xffff);
      break;
    case OPTYPE_ADDR_PC:
      r = (pc & 0xff0000) + (addr & 0xffff);
      break;
    case OPTYPE_ADDRX:
      r = (regs.db << 16) + (addr & 0xffff) + regs.x;
      break;
    case OPTYPE_ADDRY:
      r = (regs.db << 16) + (addr & 0xffff) + regs.y;
      break;
    case OPTYPE_IADDR_PC:
      r = (pc & 0xff0000) + (addr & 0xffff);
      break;
    case OPTYPE_IADDRX:
      r = (pc & 0xff0000) + ((addr + regs.x) & 0xffff);
      break;
    case OPTYPE_ILADDR:
      r = addr;
      break;
    case OPTYPE_LONG:
      r = addr;
      break;
    case OPTYPE_LONGX:
      r = (addr + regs.x);
      break;
    case OPTYPE_SR:
      r = (regs.s + (addr & 0xff)) & 0xffff;
      break;
    case OPTYPE_ISRY:
      addr = (regs.s + (addr & 0xff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr) + regs.y;
      break;
    case OPTYPE_RELB:
      r  = (pc & 0xff0000) + (((pc & 0xffff)+ 2) & 0xffff);
      r += int8(addr);
      break;
    case OPTYPE_RELW:
      r  = (pc & 0xff0000) + (((pc & 0xffff) + 3) & 0xffff);
      r += int16(addr);
      break;
  }

  return(r & 0xffffff);
}

void CPUcore::disassemble_opcode_ex(CPUcore::Opcode &opcode, uint32 addr, bool e, bool m, bool x) {
  static reg24_t pc;
  pc.d = addr;

  uint8 param[4];

  param[0] = dreadb(pc.d); pc.w++;
  param[1] = dreadb(pc.d); pc.w++;
  param[2] = dreadb(pc.d); pc.w++;
  param[3] = dreadb(pc.d);

  #define a8   (regs.e || regs.p.m)
  #define x8   (regs.e || regs.p.x)

  switch (param[0]) {
    case 0x00: opcode.set(0, OPTYPE_IM, "brk", param); break;
    case 0x01: opcode.set(0, OPTYPE_IDPX, "ora", param); break;
    case 0x02: opcode.set(0, OPTYPE_IM, "cop", param); break;
    case 0x03: opcode.set(0, OPTYPE_SR, "ora", param); break;
    case 0x04: opcode.set(0, OPTYPE_DP, "tsb", param); break;
    case 0x05: opcode.set(0, OPTYPE_DP, "ora", param); break;
    case 0x06: opcode.set(0, OPTYPE_DP, "asl", param); break;
    case 0x07: opcode.set(0, OPTYPE_ILDP, "ora", param); break;
    case 0x08: opcode.set(0, OPTYPE_IMPL, "php", param); break;
    case 0x09: if(a8)opcode.set(0, OPTYPE_IM, "ora", param, 8);
               else  opcode.set(0, OPTYPE_IM, "ora", param, 16); break;
    case 0x0a: opcode.set(0, OPTYPE_IMPL, "asl", param); break;
    case 0x0b: opcode.set(0, OPTYPE_IMPL, "phd", param); break;
    case 0x0c: opcode.set(0, OPTYPE_ADDR, "tsb", param); break;
    case 0x0d: opcode.set(0, OPTYPE_ADDR, "ora", param); break;
    case 0x0e: opcode.set(0, OPTYPE_ADDR, "asl", param); break;
    case 0x0f: opcode.set(0, OPTYPE_LONG, "ora", param); break;
    case 0x10: opcode.set(0, OPTYPE_RELB, "bpl", param); break;
    case 0x11: opcode.set(0, OPTYPE_IDPY, "ora", param); break;
    case 0x12: opcode.set(0, OPTYPE_IDP, "ora", param); break;
    case 0x13: opcode.set(0, OPTYPE_ISRY, "ora", param); break;
    case 0x14: opcode.set(0, OPTYPE_DP, "trb", param); break;
    case 0x15: opcode.set(0, OPTYPE_DPX, "ora", param); break;
    case 0x16: opcode.set(0, OPTYPE_DPX, "asl", param); break;
    case 0x17: opcode.set(0, OPTYPE_ILDPY, "ora", param); break;
    case 0x18: opcode.set(0, OPTYPE_IMPL, "clc", param); break;
    case 0x19: opcode.set(0, OPTYPE_ADDRY, "ora", param); break;
    case 0x1a: opcode.set(0, OPTYPE_IMPL, "inc", param); break;
    case 0x1b: opcode.set(0, OPTYPE_IMPL, "tcs", param); break;
    case 0x1c: opcode.set(0, OPTYPE_ADDR, "trb", param); break;
    case 0x1d: opcode.set(0, OPTYPE_ADDRX, "ora", param); break;
    case 0x1e: opcode.set(0, OPTYPE_ADDRX, "asl", param); break;
    case 0x1f: opcode.set(0, OPTYPE_LONGX, "ora", param); break;
    case 0x20: opcode.set(0, OPTYPE_ADDR_PC, "jsr", param); break;
    case 0x21: opcode.set(0, OPTYPE_IDPX, "and", param); break;
    case 0x22: opcode.set(0, OPTYPE_LONG, "jsl", param); break;
    case 0x23: opcode.set(0, OPTYPE_SR, "and", param); break;
    case 0x24: opcode.set(0, OPTYPE_DP, "bit", param); break;
    case 0x25: opcode.set(0, OPTYPE_DP, "and", param); break;
    case 0x26: opcode.set(0, OPTYPE_DP, "rol", param); break;
    case 0x27: opcode.set(0, OPTYPE_ILDP, "and", param); break;
    case 0x28: opcode.set(0, OPTYPE_IMPL, "plp", param); break;
    case 0x29: if(a8)opcode.set(0, OPTYPE_IM, "and", param, 8);
               else  opcode.set(0, OPTYPE_IM, "and", param, 16); break;
    case 0x2a: opcode.set(0, OPTYPE_IMPL, "rol", param); break;
    case 0x2b: opcode.set(0, OPTYPE_IMPL, "pld", param); break;
    case 0x2c: opcode.set(0, OPTYPE_ADDR, "bit", param); break;
    case 0x2d: opcode.set(0, OPTYPE_ADDR, "and", param); break;
    case 0x2e: opcode.set(0, OPTYPE_ADDR, "rol", param); break;
    case 0x2f: opcode.set(0, OPTYPE_LONG, "and", param); break;
    case 0x30: opcode.set(0, OPTYPE_RELB, "bmi", param); break;
    case 0x31: opcode.set(0, OPTYPE_IDPY, "and", param); break;
    case 0x32: opcode.set(0, OPTYPE_IDP, "and", param); break;
    case 0x33: opcode.set(0, OPTYPE_ISRY, "and", param); break;
    case 0x34: opcode.set(0, OPTYPE_DPX, "bit", param); break;
    case 0x35: opcode.set(0, OPTYPE_DPX, "and", param); break;
    case 0x36: opcode.set(0, OPTYPE_DPX, "rol", param); break;
    case 0x37: opcode.set(0, OPTYPE_ILDPY, "and", param); break;
    case 0x38: opcode.set(0, OPTYPE_IMPL, "sec", param); break;
    case 0x39: opcode.set(0, OPTYPE_ADDRY, "and", param); break;
    case 0x3a: opcode.set(0, OPTYPE_IMPL, "dec", param); break;
    case 0x3b: opcode.set(0, OPTYPE_IMPL, "tsc", param); break;
    case 0x3c: opcode.set(0, OPTYPE_ADDRX, "bit", param); break;
    case 0x3d: opcode.set(0, OPTYPE_ADDRX, "and", param); break;
    case 0x3e: opcode.set(0, OPTYPE_ADDRX, "rol", param); break;
    case 0x3f: opcode.set(0, OPTYPE_LONGX, "and", param); break;
    case 0x40: opcode.set(0, OPTYPE_IMPL, "rti", param); break;
    case 0x41: opcode.set(0, OPTYPE_IDPX, "eor", param); break;
    case 0x42: opcode.set(0, OPTYPE_IM, "wdm", param); break;
    case 0x43: opcode.set(0, OPTYPE_SR, "eor", param); break;
    case 0x44: opcode.set(0, OPTYPE_IM, "mvp", param); break;
    case 0x45: opcode.set(0, OPTYPE_DP, "eor", param); break;
    case 0x46: opcode.set(0, OPTYPE_DP, "lsr", param); break;
    case 0x47: opcode.set(0, OPTYPE_ILDP, "eor", param); break;
    case 0x48: opcode.set(0, OPTYPE_IMPL, "pha", param); break;
    case 0x49: if(a8)opcode.set(0, OPTYPE_IM, "eor", param, 8);
               else  opcode.set(0, OPTYPE_IM, "eor", param, 16); break;
    case 0x4a: opcode.set(0, OPTYPE_IMPL, "lsr", param); break;
    case 0x4b: opcode.set(0, OPTYPE_IMPL, "phk", param); break;
    case 0x4c: opcode.set(0, OPTYPE_ADDR_PC, "jmp", param); break;
    case 0x4d: opcode.set(0, OPTYPE_ADDR, "eor", param); break;
    case 0x4e: opcode.set(0, OPTYPE_ADDR, "lsr", param); break;
    case 0x4f: opcode.set(0, OPTYPE_LONG, "eor", param); break;
    case 0x50: opcode.set(0, OPTYPE_RELB, "bvc", param); break;
    case 0x51: opcode.set(0, OPTYPE_IDPY, "eor", param); break;
    case 0x52: opcode.set(0, OPTYPE_IDP, "eor", param); break;
    case 0x53: opcode.set(0, OPTYPE_ISRY, "eor", param); break;
    case 0x54: opcode.set(0, OPTYPE_IM, "mvn", param); break;
    case 0x55: opcode.set(0, OPTYPE_DPX, "eor", param); break;
    case 0x56: opcode.set(0, OPTYPE_DPX, "lsr", param); break;
    case 0x57: opcode.set(0, OPTYPE_ILDPY, "eor", param); break;
    case 0x58: opcode.set(0, OPTYPE_IMPL, "cli", param); break;
    case 0x59: opcode.set(0, OPTYPE_ADDRY, "eor", param); break;
    case 0x5a: opcode.set(0, OPTYPE_IMPL, "phy", param); break;
    case 0x5b: opcode.set(0, OPTYPE_IMPL, "tcd", param); break;
    case 0x5c: opcode.set(0, OPTYPE_LONG, "jml", param); break;
    case 0x5d: opcode.set(0, OPTYPE_ADDRX, "eor", param); break;
    case 0x5e: opcode.set(0, OPTYPE_ADDRX, "lsr", param); break;
    case 0x5f: opcode.set(0, OPTYPE_LONGX, "eor", param); break;
    case 0x60: opcode.set(0, OPTYPE_IMPL, "rts", param); break;
    case 0x61: opcode.set(0, OPTYPE_IDPX, "adc", param); break;
    case 0x62: opcode.set(0, OPTYPE_RELW, "per", param); break;
    case 0x63: opcode.set(0, OPTYPE_SR, "adc", param); break;
    case 0x64: opcode.set(0, OPTYPE_DP, "stz", param); break;
    case 0x65: opcode.set(0, OPTYPE_DP, "adc", param); break;
    case 0x66: opcode.set(0, OPTYPE_DP, "ror", param); break;
    case 0x67: opcode.set(0, OPTYPE_ILDP, "adc", param); break;
    case 0x68: opcode.set(0, OPTYPE_IMPL, "pla", param); break;
    case 0x69: if(a8)opcode.set(0, OPTYPE_IM, "adc", param, 8);
               else  opcode.set(0, OPTYPE_IM, "adc", param, 16); break;
    case 0x6a: opcode.set(0, OPTYPE_IMPL, "ror", param); break;
    case 0x6b: opcode.set(0, OPTYPE_IMPL, "rtl", param); break;
    case 0x6c: opcode.set(0, OPTYPE_IADDR_PC, "jmp", param); break;
    case 0x6d: opcode.set(0, OPTYPE_ADDR, "adc", param); break;
    case 0x6e: opcode.set(0, OPTYPE_ADDR, "ror", param); break;
    case 0x6f: opcode.set(0, OPTYPE_LONG, "adc", param); break;
    case 0x70: opcode.set(0, OPTYPE_RELB, "bvs", param); break;
    case 0x71: opcode.set(0, OPTYPE_IDPY, "adc", param); break;
    case 0x72: opcode.set(0, OPTYPE_IDP, "adc", param); break;
    case 0x73: opcode.set(0, OPTYPE_ISRY, "adc", param); break;
    case 0x74: opcode.set(0, OPTYPE_DPX, "stz", param); break;
    case 0x75: opcode.set(0, OPTYPE_DPX, "adc", param); break;
    case 0x76: opcode.set(0, OPTYPE_DPX, "ror", param); break;
    case 0x77: opcode.set(0, OPTYPE_ILDPY, "adc", param); break;
    case 0x78: opcode.set(0, OPTYPE_IMPL, "sei", param); break;
    case 0x79: opcode.set(0, OPTYPE_ADDRY, "adc", param); break;
    case 0x7a: opcode.set(0, OPTYPE_IMPL, "ply", param); break;
    case 0x7b: opcode.set(0, OPTYPE_IMPL, "tdc", param); break;
    case 0x7c: opcode.set(0, OPTYPE_IADDRX, "jmp", param); break;
    case 0x7d: opcode.set(0, OPTYPE_ADDRX, "adc", param); break;
    case 0x7e: opcode.set(0, OPTYPE_ADDRX, "ror", param); break;
    case 0x7f: opcode.set(0, OPTYPE_LONGX, "adc", param); break;
    case 0x80: opcode.set(0, OPTYPE_RELB, "bra", param); break;
    case 0x81: opcode.set(0, OPTYPE_IDPX, "sta", param); break;
    case 0x82: opcode.set(0, OPTYPE_RELW, "brl", param); break;
    case 0x83: opcode.set(0, OPTYPE_SR, "sta", param); break;
    case 0x84: opcode.set(0, OPTYPE_DP, "sty", param); break;
    case 0x85: opcode.set(0, OPTYPE_DP, "sta", param); break;
    case 0x86: opcode.set(0, OPTYPE_DP, "stx", param); break;
    case 0x87: opcode.set(0, OPTYPE_ILDP, "sta", param); break;
    case 0x88: opcode.set(0, OPTYPE_IMPL, "dey", param); break;
    case 0x89: if(a8)opcode.set(0, OPTYPE_IM, "bit", param, 8);
               else  opcode.set(0, OPTYPE_IM, "bit", param, 16); break;
    case 0x8a: opcode.set(0, OPTYPE_IMPL, "txa", param); break;
    case 0x8b: opcode.set(0, OPTYPE_IMPL, "phb", param); break;
    case 0x8c: opcode.set(0, OPTYPE_ADDR, "sty", param); break;
    case 0x8d: opcode.set(0, OPTYPE_ADDR, "sta", param); break;
    case 0x8e: opcode.set(0, OPTYPE_ADDR, "stx", param); break;
    case 0x8f: opcode.set(0, OPTYPE_LONG, "sta", param); break;
    case 0x90: opcode.set(0, OPTYPE_RELB, "bcc", param); break;
    case 0x91: opcode.set(0, OPTYPE_IDPY, "sta", param); break;
    case 0x92: opcode.set(0, OPTYPE_IDP, "sta", param); break;
    case 0x93: opcode.set(0, OPTYPE_ISRY, "sta", param); break;
    case 0x94: opcode.set(0, OPTYPE_DPX, "sty", param); break;
    case 0x95: opcode.set(0, OPTYPE_DPX, "sta", param); break;
    case 0x96: opcode.set(0, OPTYPE_DPY, "stx", param); break;
    case 0x97: opcode.set(0, OPTYPE_ILDPY, "sta", param); break;
    case 0x98: opcode.set(0, OPTYPE_IMPL, "tya", param); break;
    case 0x99: opcode.set(0, OPTYPE_ADDRY, "sta", param); break;
    case 0x9a: opcode.set(0, OPTYPE_IMPL, "txs", param); break;
    case 0x9b: opcode.set(0, OPTYPE_IMPL, "txy", param); break;
    case 0x9c: opcode.set(0, OPTYPE_ADDR, "stz", param); break;
    case 0x9d: opcode.set(0, OPTYPE_ADDRX, "sta", param); break;
    case 0x9e: opcode.set(0, OPTYPE_ADDRX, "stz", param); break;
    case 0x9f: opcode.set(0, OPTYPE_LONGX, "sta", param); break;
    case 0xa0: if(x8)opcode.set(0, OPTYPE_IM, "ldy", param, 8);
               else  opcode.set(0, OPTYPE_IM, "ldy", param, 16); break;
    case 0xa1: opcode.set(0, OPTYPE_IDPX, "lda", param); break;
    case 0xa2: if(x8)opcode.set(0, OPTYPE_IM, "ldx", param, 8);
               else  opcode.set(0, OPTYPE_IM, "ldx", param, 16); break;
    case 0xa3: opcode.set(0, OPTYPE_SR, "lda", param); break;
    case 0xa4: opcode.set(0, OPTYPE_DP, "ldy", param); break;
    case 0xa5: opcode.set(0, OPTYPE_DP, "lda", param); break;
    case 0xa6: opcode.set(0, OPTYPE_DP, "ldx", param); break;
    case 0xa7: opcode.set(0, OPTYPE_ILDP, "lda", param); break;
    case 0xa8: opcode.set(0, OPTYPE_IMPL, "tay", param); break;
    case 0xa9: if(a8)opcode.set(0, OPTYPE_IM, "lda", param, 8);
               else  opcode.set(0, OPTYPE_IM, "lda", param, 16); break;
    case 0xaa: opcode.set(0, OPTYPE_IMPL, "tax", param); break;
    case 0xab: opcode.set(0, OPTYPE_IMPL, "plb", param); break;
    case 0xac: opcode.set(0, OPTYPE_ADDR, "ldy", param); break;
    case 0xad: opcode.set(0, OPTYPE_ADDR, "lda", param); break;
    case 0xae: opcode.set(0, OPTYPE_ADDR, "ldx", param); break;
    case 0xaf: opcode.set(0, OPTYPE_LONG, "lda", param); break;
    case 0xb0: opcode.set(0, OPTYPE_RELB, "bcs", param); break;
    case 0xb1: opcode.set(0, OPTYPE_IDPY, "lda", param); break;
    case 0xb2: opcode.set(0, OPTYPE_IDP, "lda", param); break;
    case 0xb3: opcode.set(0, OPTYPE_ISRY, "lda", param); break;
    case 0xb4: opcode.set(0, OPTYPE_DPX, "ldy", param); break;
    case 0xb5: opcode.set(0, OPTYPE_DPX, "lda", param); break;
    case 0xb6: opcode.set(0, OPTYPE_DPY, "ldx", param); break;
    case 0xb7: opcode.set(0, OPTYPE_ILDPY, "lda", param); break;
    case 0xb8: opcode.set(0, OPTYPE_IMPL, "clv", param); break;
    case 0xb9: opcode.set(0, OPTYPE_ADDRY, "lda", param); break;
    case 0xba: opcode.set(0, OPTYPE_IMPL, "tsx", param); break;
    case 0xbb: opcode.set(0, OPTYPE_IMPL, "tyx", param); break;
    case 0xbc: opcode.set(0, OPTYPE_ADDRX, "ldy", param); break;
    case 0xbd: opcode.set(0, OPTYPE_ADDRX, "lda", param); break;
    case 0xbe: opcode.set(0, OPTYPE_ADDRY, "ldx", param); break;
    case 0xbf: opcode.set(0, OPTYPE_LONGX, "lda", param); break;
    case 0xc0: if(x8)opcode.set(0, OPTYPE_IM, "cpy", param, 8);
               else  opcode.set(0, OPTYPE_IM, "cpy", param, 16); break;
    case 0xc1: opcode.set(0, OPTYPE_IDPX, "cmp", param); break;
    case 0xc2: opcode.set(0, OPTYPE_IM, "rep", param); break;
    case 0xc3: opcode.set(0, OPTYPE_SR, "cmp", param); break;
    case 0xc4: opcode.set(0, OPTYPE_DP, "cpy", param); break;
    case 0xc5: opcode.set(0, OPTYPE_DP, "cmp", param); break;
    case 0xc6: opcode.set(0, OPTYPE_DP, "dec", param); break;
    case 0xc7: opcode.set(0, OPTYPE_ILDP, "cmp", param); break;
    case 0xc8: opcode.set(0, OPTYPE_IMPL, "iny", param); break;
    case 0xc9: if(a8)opcode.set(0, OPTYPE_IM, "cmp", param, 8);
               else  opcode.set(0, OPTYPE_IM, "cmp", param, 16); break;
    case 0xca: opcode.set(0, OPTYPE_IMPL, "dex", param); break;
    case 0xcb: opcode.set(0, OPTYPE_IMPL, "wai", param); break;
    case 0xcc: opcode.set(0, OPTYPE_ADDR, "cpy", param); break;
    case 0xcd: opcode.set(0, OPTYPE_ADDR, "cmp", param); break;
    case 0xce: opcode.set(0, OPTYPE_ADDR, "dec", param); break;
    case 0xcf: opcode.set(0, OPTYPE_LONG, "cmp", param); break;
    case 0xd0: opcode.set(0, OPTYPE_RELB, "bne", param); break;
    case 0xd1: opcode.set(0, OPTYPE_IDPY, "cmp", param); break;
    case 0xd2: opcode.set(0, OPTYPE_IDP, "cmp", param); break;
    case 0xd3: opcode.set(0, OPTYPE_ISRY, "cmp", param); break;
    case 0xd4: opcode.set(0, OPTYPE_DP, "pei", param); break;
    case 0xd5: opcode.set(0, OPTYPE_DPX, "cmp", param); break;
    case 0xd6: opcode.set(0, OPTYPE_DPX, "dec", param); break;
    case 0xd7: opcode.set(0, OPTYPE_ILDPY, "cmp", param); break;
    case 0xd8: opcode.set(0, OPTYPE_IMPL, "cld", param); break;
    case 0xd9: opcode.set(0, OPTYPE_ADDRY, "cmp", param); break;
    case 0xda: opcode.set(0, OPTYPE_IMPL, "phx", param); break;
    case 0xdb: opcode.set(0, OPTYPE_IMPL, "stp", param); break;
    case 0xdc: opcode.set(0, OPTYPE_ILADDR, "jmp", param); break;
    case 0xdd: opcode.set(0, OPTYPE_ADDRX, "cmp", param); break;
    case 0xde: opcode.set(0, OPTYPE_ADDRX, "dec", param); break;
    case 0xdf: opcode.set(0, OPTYPE_LONGX, "cmp", param); break;
    case 0xe0: if(x8)opcode.set(0, OPTYPE_IM, "cpx", param, 8);
               else  opcode.set(0, OPTYPE_IM, "cpx", param, 16); break;
    case 0xe1: opcode.set(0, OPTYPE_IDPX, "sbc", param); break;
    case 0xe2: opcode.set(0, OPTYPE_IM, "sep", param); break;
    case 0xe3: opcode.set(0, OPTYPE_SR, "sbc", param); break;
    case 0xe4: opcode.set(0, OPTYPE_DP, "cpx", param); break;
    case 0xe5: opcode.set(0, OPTYPE_DP, "sbc", param); break;
    case 0xe6: opcode.set(0, OPTYPE_DP, "inc", param); break;
    case 0xe7: opcode.set(0, OPTYPE_ILDP, "sbc", param); break;
    case 0xe8: opcode.set(0, OPTYPE_IMPL, "inx", param); break;
    case 0xe9: if(a8)opcode.set(0, OPTYPE_IM, "sbc", param, 8);
               else  opcode.set(0, OPTYPE_IM, "sbc", param, 16); break;
    case 0xea: opcode.set(0, OPTYPE_IMPL, "nop", param); break;
    case 0xeb: opcode.set(0, OPTYPE_IMPL, "xba", param); break;
    case 0xec: opcode.set(0, OPTYPE_ADDR, "cpx", param); break;
    case 0xed: opcode.set(0, OPTYPE_ADDR, "sbc", param); break;
    case 0xee: opcode.set(0, OPTYPE_ADDR, "inc", param); break;
    case 0xef: opcode.set(0, OPTYPE_LONG, "sbc", param); break;
    case 0xf0: opcode.set(0, OPTYPE_RELB, "beq", param); break;
    case 0xf1: opcode.set(0, OPTYPE_IDPY, "sbc", param); break;
    case 0xf2: opcode.set(0, OPTYPE_IDP, "sbc", param); break;
    case 0xf3: opcode.set(0, OPTYPE_ISRY, "sbc", param); break;
    case 0xf4: opcode.set(0, OPTYPE_IM, "pea", param); break;
    case 0xf5: opcode.set(0, OPTYPE_DPX, "sbc", param); break;
    case 0xf6: opcode.set(0, OPTYPE_DPX, "inc", param); break;
    case 0xf7: opcode.set(0, OPTYPE_ILDPY, "sbc", param); break;
    case 0xf8: opcode.set(0, OPTYPE_IMPL, "sed", param); break;
    case 0xf9: opcode.set(0, OPTYPE_ADDRY, "sbc", param); break;
    case 0xfa: opcode.set(0, OPTYPE_IMPL, "plx", param); break;
    case 0xfb: opcode.set(0, OPTYPE_IMPL, "xce", param); break;
    case 0xfc: opcode.set(0, OPTYPE_IADDRX, "jsr", param); break;
    case 0xfd: opcode.set(0, OPTYPE_ADDRX, "sbc", param); break;
    case 0xfe: opcode.set(0, OPTYPE_ADDRX, "inc", param); break;
    case 0xff: opcode.set(0, OPTYPE_LONGX, "sbc", param); break;

    default: opcode.set(0, OPTYPE_IMPL, "???", param); break;
  }

  #undef x8
  #undef a8
}

void CPUcore::disassemble_opcode(char *output, uint32 addr, bool hclocks) {
  static reg24_t pc;
  char t[256];
  char *s = output;

  pc.d = addr;
  sprintf(s, "%.6x ", (uint32)pc.d);

  uint8 op  = dreadb(pc.d); pc.w++;
  uint8 op0 = dreadb(pc.d); pc.w++;
  uint8 op1 = dreadb(pc.d); pc.w++;
  uint8 op2 = dreadb(pc.d);

  #define op8  ((op0))
  #define op16 ((op0) | (op1 << 8))
  #define op24 ((op0) | (op1 << 8) | (op2 << 16))
  #define a8   (regs.e || regs.p.m)
  #define x8   (regs.e || regs.p.x)

  switch(op) {
    case 0x00: sprintf(t, "brk #$%.2x              ", op8); break;
    case 0x01: sprintf(t, "ora ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0x02: sprintf(t, "cop #$%.2x              ", op8); break;
    case 0x03: sprintf(t, "ora $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0x04: sprintf(t, "tsb $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x05: sprintf(t, "ora $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x06: sprintf(t, "asl $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x07: sprintf(t, "ora [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0x08: sprintf(t, "php                   "); break;
    case 0x09: if(a8)sprintf(t, "ora #$%.2x              ", op8);
               else  sprintf(t, "ora #$%.4x            ", op16); break;
    case 0x0a: sprintf(t, "asl a                 "); break;
    case 0x0b: sprintf(t, "phd                   "); break;
    case 0x0c: sprintf(t, "tsb $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x0d: sprintf(t, "ora $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x0e: sprintf(t, "asl $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x0f: sprintf(t, "ora $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x10: sprintf(t, "bpl $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0x11: sprintf(t, "ora ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0x12: sprintf(t, "ora ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0x13: sprintf(t, "ora ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0x14: sprintf(t, "trb $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x15: sprintf(t, "ora $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x16: sprintf(t, "asl $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x17: sprintf(t, "ora [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0x18: sprintf(t, "clc                   "); break;
    case 0x19: sprintf(t, "ora $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0x1a: sprintf(t, "inc                   "); break;
    case 0x1b: sprintf(t, "tcs                   "); break;
    case 0x1c: sprintf(t, "trb $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x1d: sprintf(t, "ora $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x1e: sprintf(t, "asl $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x1f: sprintf(t, "ora $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0x20: sprintf(t, "jsr $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR_PC, op16, addr)); break;
    case 0x21: sprintf(t, "and ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0x22: sprintf(t, "jsl $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x23: sprintf(t, "and $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0x24: sprintf(t, "bit $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x25: sprintf(t, "and $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x26: sprintf(t, "rol $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x27: sprintf(t, "and [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0x28: sprintf(t, "plp                   "); break;
    case 0x29: if(a8)sprintf(t, "and #$%.2x              ", op8);
               else  sprintf(t, "and #$%.4x            ", op16); break;
    case 0x2a: sprintf(t, "rol a                 "); break;
    case 0x2b: sprintf(t, "pld                   "); break;
    case 0x2c: sprintf(t, "bit $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x2d: sprintf(t, "and $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x2e: sprintf(t, "rol $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x2f: sprintf(t, "and $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x30: sprintf(t, "bmi $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0x31: sprintf(t, "and ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0x32: sprintf(t, "and ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0x33: sprintf(t, "and ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0x34: sprintf(t, "bit $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x35: sprintf(t, "and $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x36: sprintf(t, "rol $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x37: sprintf(t, "and [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0x38: sprintf(t, "sec                   "); break;
    case 0x39: sprintf(t, "and $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0x3a: sprintf(t, "dec                   "); break;
    case 0x3b: sprintf(t, "tsc                   "); break;
    case 0x3c: sprintf(t, "bit $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x3d: sprintf(t, "and $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x3e: sprintf(t, "rol $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x3f: sprintf(t, "and $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0x40: sprintf(t, "rti                   "); break;
    case 0x41: sprintf(t, "eor ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0x42: sprintf(t, "wdm #$%.2x              ", op8); break;
    case 0x43: sprintf(t, "eor $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0x44: sprintf(t, "mvp $%.2x,$%.2x           ", op1, op8); break;
    case 0x45: sprintf(t, "eor $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x46: sprintf(t, "lsr $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x47: sprintf(t, "eor [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0x48: sprintf(t, "pha                   "); break;
    case 0x49: if(a8)sprintf(t, "eor #$%.2x              ", op8);
               else  sprintf(t, "eor #$%.4x            ", op16); break;
    case 0x4a: sprintf(t, "lsr a                 "); break;
    case 0x4b: sprintf(t, "phk                   "); break;
    case 0x4c: sprintf(t, "jmp $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR_PC, op16, addr)); break;
    case 0x4d: sprintf(t, "eor $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x4e: sprintf(t, "lsr $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x4f: sprintf(t, "eor $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x50: sprintf(t, "bvc $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0x51: sprintf(t, "eor ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0x52: sprintf(t, "eor ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0x53: sprintf(t, "eor ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0x54: sprintf(t, "mvn $%.2x,$%.2x           ", op1, op8); break;
    case 0x55: sprintf(t, "eor $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x56: sprintf(t, "lsr $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x57: sprintf(t, "eor [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0x58: sprintf(t, "cli                   "); break;
    case 0x59: sprintf(t, "eor $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0x5a: sprintf(t, "phy                   "); break;
    case 0x5b: sprintf(t, "tcd                   "); break;
    case 0x5c: sprintf(t, "jml $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x5d: sprintf(t, "eor $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x5e: sprintf(t, "lsr $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x5f: sprintf(t, "eor $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0x60: sprintf(t, "rts                   "); break;
    case 0x61: sprintf(t, "adc ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0x62: sprintf(t, "per $%.4x             ", uint16(decode(OPTYPE_RELW, op16, addr))); break;
    case 0x63: sprintf(t, "adc $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0x64: sprintf(t, "stz $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x65: sprintf(t, "adc $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x66: sprintf(t, "ror $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x67: sprintf(t, "adc [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0x68: sprintf(t, "pla                   "); break;
    case 0x69: if(a8)sprintf(t, "adc #$%.2x              ", op8);
               else  sprintf(t, "adc #$%.4x            ", op16); break;
    case 0x6a: sprintf(t, "ror a                 "); break;
    case 0x6b: sprintf(t, "rtl                   "); break;
    case 0x6c: sprintf(t, "jmp ($%.4x)   [%.6x]", op16, decode(OPTYPE_IADDR_PC, op16, addr)); break;
    case 0x6d: sprintf(t, "adc $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x6e: sprintf(t, "ror $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x6f: sprintf(t, "adc $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x70: sprintf(t, "bvs $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0x71: sprintf(t, "adc ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0x72: sprintf(t, "adc ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0x73: sprintf(t, "adc ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0x74: sprintf(t, "stz $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x75: sprintf(t, "adc $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x76: sprintf(t, "ror $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x77: sprintf(t, "adc [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0x78: sprintf(t, "sei                   "); break;
    case 0x79: sprintf(t, "adc $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0x7a: sprintf(t, "ply                   "); break;
    case 0x7b: sprintf(t, "tdc                   "); break;
    case 0x7c: sprintf(t, "jmp ($%.4x,x) [%.6x]", op16, decode(OPTYPE_IADDRX, op16, addr)); break;
    case 0x7d: sprintf(t, "adc $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x7e: sprintf(t, "ror $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x7f: sprintf(t, "adc $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0x80: sprintf(t, "bra $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0x81: sprintf(t, "sta ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0x82: sprintf(t, "brl $%.4x     [%.6x]", uint16(decode(OPTYPE_RELW, op16, addr)), decode(OPTYPE_RELW, op16, addr)); break;
    case 0x83: sprintf(t, "sta $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0x84: sprintf(t, "sty $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x85: sprintf(t, "sta $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x86: sprintf(t, "stx $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0x87: sprintf(t, "sta [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0x88: sprintf(t, "dey                   "); break;
    case 0x89: if(a8)sprintf(t, "bit #$%.2x              ", op8);
               else  sprintf(t, "bit #$%.4x            ", op16); break;
    case 0x8a: sprintf(t, "txa                   "); break;
    case 0x8b: sprintf(t, "phb                   "); break;
    case 0x8c: sprintf(t, "sty $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x8d: sprintf(t, "sta $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x8e: sprintf(t, "stx $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x8f: sprintf(t, "sta $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0x90: sprintf(t, "bcc $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0x91: sprintf(t, "sta ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0x92: sprintf(t, "sta ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0x93: sprintf(t, "sta ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0x94: sprintf(t, "sty $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x95: sprintf(t, "sta $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0x96: sprintf(t, "stx $%.2x,y     [%.6x]", op8, decode(OPTYPE_DPY, op8, addr)); break;
    case 0x97: sprintf(t, "sta [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0x98: sprintf(t, "tya                   "); break;
    case 0x99: sprintf(t, "sta $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0x9a: sprintf(t, "txs                   "); break;
    case 0x9b: sprintf(t, "txy                   "); break;
    case 0x9c: sprintf(t, "stz $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0x9d: sprintf(t, "sta $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x9e: sprintf(t, "stz $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0x9f: sprintf(t, "sta $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0xa0: if(x8)sprintf(t, "ldy #$%.2x              ", op8);
               else  sprintf(t, "ldy #$%.4x            ", op16); break;
    case 0xa1: sprintf(t, "lda ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0xa2: if(x8)sprintf(t, "ldx #$%.2x              ", op8);
               else  sprintf(t, "ldx #$%.4x            ", op16); break;
    case 0xa3: sprintf(t, "lda $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0xa4: sprintf(t, "ldy $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xa5: sprintf(t, "lda $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xa6: sprintf(t, "ldx $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xa7: sprintf(t, "lda [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0xa8: sprintf(t, "tay                   "); break;
    case 0xa9: if(a8)sprintf(t, "lda #$%.2x              ", op8);
               else  sprintf(t, "lda #$%.4x            ", op16); break;
    case 0xaa: sprintf(t, "tax                   "); break;
    case 0xab: sprintf(t, "plb                   "); break;
    case 0xac: sprintf(t, "ldy $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xad: sprintf(t, "lda $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xae: sprintf(t, "ldx $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xaf: sprintf(t, "lda $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0xb0: sprintf(t, "bcs $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0xb1: sprintf(t, "lda ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0xb2: sprintf(t, "lda ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0xb3: sprintf(t, "lda ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0xb4: sprintf(t, "ldy $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0xb5: sprintf(t, "lda $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0xb6: sprintf(t, "ldx $%.2x,y     [%.6x]", op8, decode(OPTYPE_DPY, op8, addr)); break;
    case 0xb7: sprintf(t, "lda [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0xb8: sprintf(t, "clv                   "); break;
    case 0xb9: sprintf(t, "lda $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0xba: sprintf(t, "tsx                   "); break;
    case 0xbb: sprintf(t, "tyx                   "); break;
    case 0xbc: sprintf(t, "ldy $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0xbd: sprintf(t, "lda $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0xbe: sprintf(t, "ldx $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0xbf: sprintf(t, "lda $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0xc0: if(x8)sprintf(t, "cpy #$%.2x              ", op8);
               else  sprintf(t, "cpy #$%.4x            ", op16); break;
    case 0xc1: sprintf(t, "cmp ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0xc2: sprintf(t, "rep #$%.2x              ", op8); break;
    case 0xc3: sprintf(t, "cmp $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0xc4: sprintf(t, "cpy $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xc5: sprintf(t, "cmp $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xc6: sprintf(t, "dec $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xc7: sprintf(t, "cmp [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0xc8: sprintf(t, "iny                   "); break;
    case 0xc9: if(a8)sprintf(t, "cmp #$%.2x              ", op8);
               else  sprintf(t, "cmp #$%.4x            ", op16); break;
    case 0xca: sprintf(t, "dex                   "); break;
    case 0xcb: sprintf(t, "wai                   "); break;
    case 0xcc: sprintf(t, "cpy $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xcd: sprintf(t, "cmp $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xce: sprintf(t, "dec $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xcf: sprintf(t, "cmp $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0xd0: sprintf(t, "bne $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0xd1: sprintf(t, "cmp ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0xd2: sprintf(t, "cmp ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0xd3: sprintf(t, "cmp ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0xd4: sprintf(t, "pei ($%.2x)     [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xd5: sprintf(t, "cmp $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0xd6: sprintf(t, "dec $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0xd7: sprintf(t, "cmp [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0xd8: sprintf(t, "cld                   "); break;
    case 0xd9: sprintf(t, "cmp $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0xda: sprintf(t, "phx                   "); break;
    case 0xdb: sprintf(t, "stp                   "); break;
    case 0xdc: sprintf(t, "jmp [$%.4x]   [%.6x]", op16, decode(OPTYPE_ILADDR, op16, addr)); break;
    case 0xdd: sprintf(t, "cmp $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0xde: sprintf(t, "dec $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0xdf: sprintf(t, "cmp $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
    case 0xe0: if(x8)sprintf(t, "cpx #$%.2x              ", op8);
               else  sprintf(t, "cpx #$%.4x            ", op16); break;
    case 0xe1: sprintf(t, "sbc ($%.2x,x)   [%.6x]", op8, decode(OPTYPE_IDPX, op8, addr)); break;
    case 0xe2: sprintf(t, "sep #$%.2x              ", op8); break;
    case 0xe3: sprintf(t, "sbc $%.2x,s     [%.6x]", op8, decode(OPTYPE_SR, op8, addr)); break;
    case 0xe4: sprintf(t, "cpx $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xe5: sprintf(t, "sbc $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xe6: sprintf(t, "inc $%.2x       [%.6x]", op8, decode(OPTYPE_DP, op8, addr)); break;
    case 0xe7: sprintf(t, "sbc [$%.2x]     [%.6x]", op8, decode(OPTYPE_ILDP, op8, addr)); break;
    case 0xe8: sprintf(t, "inx                   "); break;
    case 0xe9: if(a8)sprintf(t, "sbc #$%.2x              ", op8);
               else  sprintf(t, "sbc #$%.4x            ", op16); break;
    case 0xea: sprintf(t, "nop                   "); break;
    case 0xeb: sprintf(t, "xba                   "); break;
    case 0xec: sprintf(t, "cpx $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xed: sprintf(t, "sbc $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xee: sprintf(t, "inc $%.4x     [%.6x]", op16, decode(OPTYPE_ADDR, op16, addr)); break;
    case 0xef: sprintf(t, "sbc $%.6x   [%.6x]", op24, decode(OPTYPE_LONG, op24, addr)); break;
    case 0xf0: sprintf(t, "beq $%.4x     [%.6x]", uint16(decode(OPTYPE_RELB, op8, addr)), decode(OPTYPE_RELB, op8, addr)); break;
    case 0xf1: sprintf(t, "sbc ($%.2x),y   [%.6x]", op8, decode(OPTYPE_IDPY, op8, addr)); break;
    case 0xf2: sprintf(t, "sbc ($%.2x)     [%.6x]", op8, decode(OPTYPE_IDP, op8, addr)); break;
    case 0xf3: sprintf(t, "sbc ($%.2x,s),y [%.6x]", op8, decode(OPTYPE_ISRY, op8, addr)); break;
    case 0xf4: sprintf(t, "pea $%.4x             ", op16); break;
    case 0xf5: sprintf(t, "sbc $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0xf6: sprintf(t, "inc $%.2x,x     [%.6x]", op8, decode(OPTYPE_DPX, op8, addr)); break;
    case 0xf7: sprintf(t, "sbc [$%.2x],y   [%.6x]", op8, decode(OPTYPE_ILDPY, op8, addr)); break;
    case 0xf8: sprintf(t, "sed                   "); break;
    case 0xf9: sprintf(t, "sbc $%.4x,y   [%.6x]", op16, decode(OPTYPE_ADDRY, op16, addr)); break;
    case 0xfa: sprintf(t, "plx                   "); break;
    case 0xfb: sprintf(t, "xce                   "); break;
    case 0xfc: sprintf(t, "jsr ($%.4x,x) [%.6x]", op16, decode(OPTYPE_IADDRX, op16, addr)); break;
    case 0xfd: sprintf(t, "sbc $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0xfe: sprintf(t, "inc $%.4x,x   [%.6x]", op16, decode(OPTYPE_ADDRX, op16, addr)); break;
    case 0xff: sprintf(t, "sbc $%.6x,x [%.6x]", op24, decode(OPTYPE_LONGX, op24, addr)); break;
  }

  #undef op8
  #undef op16
  #undef op24
  #undef a8
  #undef x8

  strcat(s, t);
  strcat(s, " ");

  sprintf(t, "A:%.4x X:%.4x Y:%.4x S:%.4x D:%.4x DB:%.2x ",
    regs.a.w, regs.x.w, regs.y.w, regs.s.w, regs.d.w, regs.db);
  strcat(s, t);

  if(regs.e) {
    sprintf(t, "%c%c%c%c%c%c%c%c",
      regs.p.n ? 'N' : 'n', regs.p.v ? 'V' : 'v',
      regs.p.m ? '1' : '0', regs.p.x ? 'B' : 'b',
      regs.p.d ? 'D' : 'd', regs.p.i ? 'I' : 'i',
      regs.p.z ? 'Z' : 'z', regs.p.c ? 'C' : 'c');
  } else {
    sprintf(t, "%c%c%c%c%c%c%c%c",
      regs.p.n ? 'N' : 'n', regs.p.v ? 'V' : 'v',
      regs.p.m ? 'M' : 'm', regs.p.x ? 'X' : 'x',
      regs.p.d ? 'D' : 'd', regs.p.i ? 'I' : 'i',
      regs.p.z ? 'Z' : 'z', regs.p.c ? 'C' : 'c');
  }

  strcat(s, t);
  strcat(s, " ");

  if (hclocks)
    sprintf(t, "V:%3d H:%4d F:%2d", cpu.vcounter(), cpu.hcounter(), cpu.framecounter());
  else
    sprintf(t, "V:%3d H:%3d F:%2d", cpu.vcounter(), cpu.hdot(), cpu.framecounter());
  strcat(s, t);
}

#endif
