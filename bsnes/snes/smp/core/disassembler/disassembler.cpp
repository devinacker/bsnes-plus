#ifdef SMPCORE_CPP

uint16 SMPcore::relb(int8 offset, int op_len, uint16 pc) {
  return pc + op_len + offset;
}

uint8 SMPcore::dreadb(uint16 addr) {
  SNES::debugger.bus_access = true;
  return smp.op_debugread(addr);
  SNES::debugger.bus_access = false;
}

uint16 SMPcore::dreadw(uint16 addr) {
  uint16 r;
  r  = dreadb(addr + 0) <<  0;
  r |= dreadb(addr + 1) <<  8;
  return r;
}

uint16 SMPcore::decode(uint8 offset_type, uint16 addr, uint16 pc) {
  uint16 r = 0;
  uint16 dp = regs.p.p << 8;

  switch(offset_type) {
  case SNESSMP::Direct:
    r = dp | (addr & 0xff);
    break;
  case SNESSMP::DirectIndX:
  case SNESSMP::DirectIndXA:
  case SNESSMP::DirectIndXY:
  case SNESSMP::ADirectIndX:
  case SNESSMP::YDirectIndX:
    r = dp | ((addr + regs.x) & 0xff);
    break;
  case SNESSMP::DirectIndYX:
  case SNESSMP::XDirectIndY:
    r = dp | ((addr + regs.y) & 0xff);
    break;
  case SNESSMP::Absolute:
    r = addr;
    break;
  case SNESSMP::AbsoluteXA:
  case SNESSMP::AAbsoluteX:
    r = addr + regs.x;
    break;
  case SNESSMP::AbsoluteYA:
  case SNESSMP::AAbsoluteY:
    r = addr + regs.y;
    break;
  case SNESSMP::AIDirectIndX:
  case SNESSMP::IDirectIndXA:
    r = dreadw(dp | ((addr + regs.x) & 0xff));
    break;
  case SNESSMP::AIDirectIndY:
  case SNESSMP::IDirectIndYA:
    r = dreadw(dp | (addr & 0xff)) + regs.y;
    break;
  case SNESSMP::IAbsoluteX:
    r = dreadw(addr + regs.x);
    break;
  case SNESSMP::Relative:
  case SNESSMP::YRelative:
    r = relb((int8)addr, 2, pc);
    break;
  case SNESSMP::DirectRelative:
  case SNESSMP::DirectIndXRelative:
    r = relb((int8)addr, 3, pc);
    break;
  case SNESSMP::PVector:
    r = 0xff00 | (addr & 0xff);
    break;
  case SNESSMP::TVector:
    addr = dreadb(pc) >> 4;
    addr = 0xffde - (addr << 1);
    r = dreadw(addr);
    break;
  }

  return r;
}

void SMPcore::disassemble_opcode_ex(SMPcore::Opcode &opcode, uint16 addr) {
  uint8 param[3];

  SNES::debugger.bus_access = true;
  param[0] = smp.op_debugread(addr + 0);
  param[1] = smp.op_debugread(addr + 1);
  param[2] = smp.op_debugread(addr + 2);
  SNES::debugger.bus_access = false;

  const SNESSMP::OpcodeInfo& op = smpOpcodeInfo[param[0]];
  opcode.set(0, op.mode, op.name, param, SNESSMP::getOpcodeLength(param[0]) - 1);

  switch (param[0]) {
    case 0x01: case 0x04:
    case 0x10: case 0x11: case 0x13:
    case 0x21: case 0x23: case 0x2e:
    case 0x30: case 0x31: case 0x33: case 0x3f:
    case 0x41: case 0x43: case 0x4f:
    case 0x50: case 0x51: case 0x53:
    case 0x61: case 0x63: case 0x6e:
    case 0x70: case 0x71: case 0x73:
    case 0x81: case 0x83:
    case 0x90: case 0x91: case 0x93:
    case 0xa1: case 0xa3:
    case 0xb0: case 0xb1: case 0xb3:
    case 0xc1: case 0xc3:
    case 0xd0: case 0xd1: case 0xd3: case 0xde:
    case 0xe1: case 0xe3:
    case 0xf0: case 0xf1: case 0xf3: case 0xfe:
      opcode.flags |= Opcode::FLAG_BRA_CONTINUE; break;
    case 0x0d:
      opcode.flags |= Opcode::FLAG_PUSH_P; break;
    case 0x0f:
      opcode.flags |= Opcode::FLAG_BRK; break;
    case 0x1f: case 0x2f: case 0x5f:
      opcode.flags |= Opcode::FLAG_BRA; break;
    case 0x20:
      opcode.flags |= Opcode::FLAG_RESET_P; break;
    case 0x40:
      opcode.flags |= Opcode::FLAG_SET_P; break;
    case 0x6f: case 0x7f:
      opcode.flags |= Opcode::FLAG_RETURN; break;
    case 0x8e:
      opcode.flags |= Opcode::FLAG_POP_P; break;
    case 0xef: case 0xff:
      opcode.flags |= Opcode::FLAG_HALT; break;
  }

  if (SNESSMP::getOpcodeIndirect(param[0])) {
    opcode.flags |= Opcode::FLAG_INDIRECT;
  }
}

void SMPcore::disassemble_opcode(char *output, uint16 addr) {
  char *s, t[512];
  uint8  op, op0, op1;
  s = output;

  sprintf(s, "..%.4x ", addr);
  
  SNES::debugger.bus_access = true;
  op  = smp.op_debugread(addr + 0);
  op0 = smp.op_debugread(addr + 1);
  op1 = smp.op_debugread(addr + 2);
  SNES::debugger.bus_access = false;
  
  sprintf(t, "%-23s ", SNESSMP::disassemble(addr, op, op0, op1, regs.p.p)());
  strcat(s, t);

  sprintf(t, "A:%.2x X:%.2x Y:%.2x SP:01%.2x YA:%.4x ",
    regs.a, regs.x, regs.y, regs.sp, (uint16)regs.ya);
  strcat(s, t);

  sprintf(t, "%c%c%c%c%c%c%c%c",
    regs.p.n ? 'N' : '.',
    regs.p.v ? 'V' : '.',
    regs.p.p ? 'P' : '.',
    regs.p.b ? 'B' : '.',
    regs.p.h ? 'H' : '.',
    regs.p.i ? 'I' : '.',
    regs.p.z ? 'Z' : '.',
    regs.p.c ? 'C' : '.');
  strcat(s, t);
}

#endif
