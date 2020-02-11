#ifdef SUPERGAMEBOY_CPP

uint16 SGBDebugger::relb(int8 offset, int op_len, uint16 pc) {
  return pc + op_len + offset;
}

uint8 SGBDebugger::dreadb(uint16 addr) {
  SNES::debugger.bus_access = true;
  return supergameboy.read_gb(addr);
  SNES::debugger.bus_access = false;
}

uint16 SGBDebugger::dreadw(uint16 addr) {
  uint16 r;
  r  = dreadb(addr + 0) <<  0;
  r |= dreadb(addr + 1) <<  8;
  return r;
}

uint16 SGBDebugger::decode(uint8 offset_type, uint16 addr, uint16 pc) {
  uint16 r = 0;
  uint16 dp = 0xff00;

  switch(offset_type) {
  case GBCPU::Addr8Ptr:
    r = dp | (uint8)addr;
    break;
  case GBCPU::Addr16:
  case GBCPU::Addr16Ptr:
    r = addr;
    break;
  case GBCPU::RegBCPtr:
    r = getRegister(RegisterBC);
    break;
  case GBCPU::RegCPtr:
    r = dp | (uint8)getRegister(RegisterBC);
    break;
  case GBCPU::RegDEPtr:
    r = getRegister(RegisterDE);
    break;
  case GBCPU::RegHLPtr:
  case GBCPU::RegHLPtrInc:
  case GBCPU::RegHLPtrDec:
  case GBCPU::PrefixCB:
    r = getRegister(RegisterHL);
    break;
  case GBCPU::PCRelative:
    r = relb((int8)addr, 2, pc);
    break;
  case GBCPU::SPRelative:
    r = relb((int8)addr, 2, getRegister(RegisterSP));
    break;
  case GBCPU::RST:
    r = dreadb(pc) & 0x38;
    break;
  }

  return r;
}

void SGBDebugger::disassemble_opcode_ex(SGBDebugger::Opcode &opcode, uint16 addr) {
  uint8 param[3];

  SNES::debugger.bus_access = true;
  param[0] = read_gb(addr + 0);
  param[1] = read_gb(addr + 1);
  param[2] = read_gb(addr + 2);
  SNES::debugger.bus_access = false;

  const GBCPU::OpcodeInfo& op = gbOpcodeInfo[param[0]];
  opcode.set(0, op.mode0, op.mode1, op.name, param, GBCPU::getOpcodeLength(param[0]) - 1);

  switch (param[0]) {
    case 0x18: case 0xc3: case 0xe9:
      opcode.flags |= Opcode::FLAG_BRA; break;
    case 0x20: case 0x28: case 0x30: case 0x38:
    case 0xc2: case 0xc4: case 0xc7: case 0xcc:
    case 0xcd: case 0xcf: case 0xd4: case 0xd7:
    case 0xdc: case 0xdf: case 0xe7: case 0xef:
    case 0xf7: case 0xff:
      opcode.flags |= Opcode::FLAG_BRA_CONTINUE; break;
    case 0xc9: case 0xd9:
      opcode.flags |= Opcode::FLAG_RETURN; break;
    case 0xf1:
      opcode.flags |= Opcode::FLAG_POP_F; break;
    case 0xf5:
      opcode.flags |= Opcode::FLAG_PUSH_F; break;
  }

  if (GBCPU::getOpcodeIndirect(param[0], param[1])) {
    opcode.flags |= Opcode::FLAG_INDIRECT;
  }
}

void SGBDebugger::disassemble_opcode(char *output, uint16_t addr) {
  char t[256];
  char *s = output;

  sprintf(s, "..%.4x ", addr);

  uint8 op  = read_gb(addr);
  uint8 op0 = read_gb(addr + 1);
  uint8 op1 = read_gb(addr + 2);
  
  sprintf(t, "%-23s ", nall::GBCPU::disassemble(addr, op, op0, op1)());
  strcat(s, t);
  
  uint16_t af = getRegister(RegisterAF);
  uint16_t bc = getRegister(RegisterBC);
  uint16_t de = getRegister(RegisterDE);
  uint16_t hl = getRegister(RegisterHL);
  uint16_t sp = getRegister(RegisterSP);
  sprintf(t, "AF:%.4x BC:%.4x DE:%.4x HL:%.4x SP:%.4x ", af, bc, de, hl, sp);
  strcat(s, t);
  
  sprintf(t, "%c%c%c%c ",
    (af & 0x80) ? 'Z' : '.', (af & 0x40) ? 'N' : '.',
    (af & 0x20) ? 'H' : '.', (af & 0x10) ? 'C' : '.');
  strcat(s, t);
}

#endif
