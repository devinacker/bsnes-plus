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
    case SNESCPU::Direct:
      r = (regs.d + (addr & 0xff)) & 0xffff;
      break;
    case SNESCPU::DirectX:
      r = (regs.d + regs.x + (addr & 0xff)) & 0xffff;
      break;
    case SNESCPU::DirectY:
      r = (regs.d + regs.y + (addr & 0xff)) & 0xffff;
      break;
    case SNESCPU::IDirect:
      addr = (regs.d + (addr & 0xff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr);
      break;
    case SNESCPU::IDirectX:
      addr = (regs.d + regs.x + (addr & 0xff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr);
      break;
    case SNESCPU::IDirectY:
      addr = (regs.d + (addr & 0xff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr) + regs.y;
      break;
    case SNESCPU::ILDirect:
      addr = (regs.d + (addr & 0xff)) & 0xffff;
      r = dreadl(addr);
      break;
    case SNESCPU::ILDirectY:
      addr = (regs.d + (addr & 0xff)) & 0xffff;
      r = dreadl(addr) + regs.y;
      break;
    case SNESCPU::Address:
      r = (regs.db << 16) + (addr & 0xffff);
      break;
    case SNESCPU::PAddress:
      r = (pc & 0xff0000) + (addr & 0xffff);
      break;
    case SNESCPU::AddressX:
      r = (regs.db << 16) + (addr & 0xffff) + regs.x;
      break;
    case SNESCPU::AddressY:
      r = (regs.db << 16) + (addr & 0xffff) + regs.y;
      break;
    case SNESCPU::PIAddress:
      r = (pc & 0xff0000) + (addr & 0xffff);
      break;
    case SNESCPU::IAddressX:
      r = (pc & 0xff0000) + ((addr + regs.x) & 0xffff);
      break;
    case SNESCPU::ILAddress:
      r = addr;
      break;
    case SNESCPU::Long:
      r = addr;
      break;
    case SNESCPU::LongX:
      r = (addr + regs.x);
      break;
    case SNESCPU::Stack:
      r = (regs.s + (addr & 0xff)) & 0xffff;
      break;
    case SNESCPU::IStackY:
      addr = (regs.s + (addr & 0xff)) & 0xffff;
      r = (regs.db << 16) + dreadw(addr) + regs.y;
      break;
    case SNESCPU::RelativeShort:
      r  = (pc & 0xff0000) + (((pc & 0xffff)+ 2) & 0xffff);
      r += int8(addr);
      break;
    case SNESCPU::RelativeLong:
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

  const SNESCPU::OpcodeInfo& op = cpuOpcodeInfo[param[0]];
  opcode.set(0, op.mode, op.name, param, 
             SNESCPU::getOpcodeLength(e || m, e || x, param[0]) - 1);

  switch (param[0]) {
    case 0x00:
      opcode.flags |= Opcode::FLAG_BRK; break;
    case 0x08: 
      opcode.flags |= Opcode::FLAG_PUSH_P; break;
    case 0x10: case 0x20: case 0x22: case 0x30:
    case 0x50: case 0x70: case 0x90: case 0xb0:
    case 0xd0: case 0xf0: case 0xfc:
      opcode.flags |= Opcode::FLAG_BRA_CONTINUE; break;
    case 0x28:
      opcode.flags |= Opcode::FLAG_POP_P; break;
    case 0x40: case 0x60: case 0x6b:
      opcode.flags |= Opcode::FLAG_RETURN; break;
    case 0x4c: case 0x5c: case 0x6c: case 0x7c:
    case 0x80: case 0x82: case 0xdc:
      opcode.flags |= Opcode::FLAG_BRA; break;
    case 0xc2:
      if (opcode.op8() & 0x10) opcode.flags |= Opcode::FLAG_RESET_X;
      if (opcode.op8() & 0x20) opcode.flags |= Opcode::FLAG_RESET_M;
      break;
    case 0xdb:
      opcode.flags |= Opcode::FLAG_HALT; break;
    case 0xe2:
      if (opcode.op8() & 0x10) opcode.flags |= Opcode::FLAG_SET_X;
      if (opcode.op8() & 0x20) opcode.flags |= Opcode::FLAG_SET_M;
      break;
    case 0xfb:
      opcode.flags |= Opcode::FLAG_RESET_E; break;
  }

  if (SNESCPU::getOpcodeIndirect(param[0])) {
    opcode.flags |= Opcode::FLAG_INDIRECT;
  }
}

// create a human-readable text version of the current opcode
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
  
  #define op24 ((op0) | (op1 << 8) | (op2 << 16))
  #define a8   (regs.e || regs.p.m)
  #define x8   (regs.e || regs.p.x)

  unsigned op_mode = cpuOpcodeInfo[op].mode;
  unsigned op_len  = SNESCPU::getOpcodeLength(a8, x8, op);

  sprintf(t, "%-14s ", SNESCPU::disassemble(addr, a8, x8, op, op0, op1, op2)());
  strcat(s, t);
  
  if (op_mode < SNESCPU::Direct || op_mode == SNESCPU::BlockMove)
    sprintf(t, "         ");
  else
    sprintf(t, "[%.6x] ", decode(op_mode, op24, addr));
  strcat(s, t);
  
  #undef op24
  #undef a8
  #undef x8

  sprintf(t, "A:%.4x X:%.4x Y:%.4x S:%.4x D:%.4x DB:%.2x ",
    regs.a.w, regs.x.w, regs.y.w, regs.s.w, regs.d.w, regs.db);
  strcat(s, t);

  if(regs.e) {
    sprintf(t, "%c%c%c%c%c%c%c%c ",
      regs.p.n ? 'N' : '.', regs.p.v ? 'V' : '.',
      regs.p.m ? '1' : '.', regs.p.x ? 'B' : '.',
      regs.p.d ? 'D' : '.', regs.p.i ? 'I' : '.',
      regs.p.z ? 'Z' : '.', regs.p.c ? 'C' : '.');
  } else {
    sprintf(t, "%c%c%c%c%c%c%c%c ",
      regs.p.n ? 'N' : '.', regs.p.v ? 'V' : '.',
      regs.p.m ? 'M' : '.', regs.p.x ? 'X' : '.',
      regs.p.d ? 'D' : '.', regs.p.i ? 'I' : '.',
      regs.p.z ? 'Z' : '.', regs.p.c ? 'C' : '.');
  }
  strcat(s, t);

  if (hclocks)
    sprintf(t, "V:%3d H:%4d F:%2d", cpu.vcounter(), cpu.hcounter(), cpu.framecounter());
  else
    sprintf(t, "V:%3d H:%3d F:%2d", cpu.vcounter(), cpu.hdot(), cpu.framecounter());
  strcat(s, t);
}

// disassemble current opcode but represent as a binary format instead of text
//
// goals:
// 1) be really fast
// 2) be fixed length and easy to parse out later
// 3) be as compact as possible. (abridgedFormat cuts even more stuff)
// 4) be extensible (use a header length so we can add more info later)
void CPUcore::disassemble_opcode_bin(char* buf, uint32 addr, int &len_out, bool abridgedFormat) {
    static reg24_t pc;

    pc.d = addr;
    uint8 opcode = dreadb(pc.d);
    unsigned opcode_len = SNESCPU::getOpcodeLength((regs.e || regs.p.m), (regs.e || regs.p.x), opcode);

    int i = 0;

    // --- header (2 bytes) ---

    // watermark identifying the type of data coming next
    buf[i++] = abridgedFormat ? 0xEE : 0xEF;

    // size in bytes of data starting after this byte (we will populate final size at the end)
    int sizeIdx = i; i++;

    // --- data ---
    buf[i++] = (addr >> 0) & 0xFF;
    buf[i++] = (addr >> 8) & 0xFF;
    buf[i++] = (addr >> 16) & 0xFF;

    // # of bytes for the instruction we're looking at
    buf[i++] = opcode_len; // valid values: 1,2,3,4

    buf[i++] = (regs.d.w >> 0) & 0xFF;
    buf[i++] = (regs.d.w >> 8) & 0xFF;

    buf[i++] = (regs.db) & 0xFF;

    buf[i++] = (regs.p) & 0xFF; // 8 flags stored as bitmask in 1 byte

    if (!abridgedFormat) {
        // we'll always transmit 4 bytes, but, consumers should only use up to 'opcode_len'
        // and discard the remaining bytes.
        // i.e. if opcode_len is 2,
        // then a consumer should USE opcode and operand0 (2 bytes)
        // then read but discard the remaining 2 bytes (operand1 and operand2 will be garbage)
        pc.w++;
        uint8 operand0 = dreadb(pc.d); pc.w++;
        uint8 operand1 = dreadb(pc.d); pc.w++;
        uint8 operand2 = dreadb(pc.d);

        buf[i++] = opcode;      // always valid (opcode_len >= 1)
        buf[i++] = operand0;    // valid if opcode_len >= 2
        buf[i++] = operand1;    // valid if opcode_len >= 3
        buf[i++] = operand2;    // valid if opcode_len == 4

        buf[i++] = (regs.a.w >> 0) & 0xFF;
        buf[i++] = (regs.a.w >> 8) & 0xFF;

        buf[i++] = (regs.x.w >> 0) & 0xFF;
        buf[i++] = (regs.x.w >> 8) & 0xFF;

        buf[i++] = (regs.y.w >> 0) & 0xFF;
        buf[i++] = (regs.y.w >> 8) & 0xFF;

        buf[i++] = (regs.s.w >> 0) & 0xFF;
        buf[i++] = (regs.s.w >> 8) & 0xFF;

        buf[i++] = (regs.e) & 0xFF; // emu flag

        // TODO: hclocks/etc if we want them.
    }

    // put the length of everything back in the header
    len_out = i;
    buf[sizeIdx] = len_out - sizeIdx - 1;
}

#endif