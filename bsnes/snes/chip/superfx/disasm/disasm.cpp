#ifdef SUPERFX_CPP

#define case4(id) \
  case id+ 0: case id+ 1: case id+ 2: case id+ 3
#define case6(id) \
  case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5
#define case12(id) \
  case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
  case id+ 8: case id+ 9: case id+10: case id+11
#define case15(id) \
  case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
  case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14
#define case16(id) \
  case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
  case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: case id+15

uint32 SuperFX::decode(uint8 offset_type, uint32 addr, uint32 pc) {
  uint32 r = 0;

  switch(offset_type) {
    case ImpliedLoop:
      r = (pc & 0xff0000) | regs.r[13];
      break;
    case ImpliedROM:
      r = (regs.rombr << 16) + regs.r[14];
      break;
    case ImpliedRAM:
      r = 0x700000 + (regs.rambr << 16) + regs.ramaddr;
      break;
    case Relative: // addr is offset in second byte
      r = (pc & 0xff0000) + (((pc & 0xffff) + 2 + (int8)addr) & 0xffff);
      break;
    case LinkRelative: // addr is offset in low nibble of instruction
      r = (pc & 0xff0000) + (((pc & 0xffff) + 1 + addr) & 0xffff);
      break;
    case RegisterJump: // addr is register no. in low nibble of instruction
      r = (pc & 0xff0000) | regs.r[addr];
      break;
    case RegisterLJump: // addr is register no. in low nibble of instruction
      r = (regs.r[addr] << 16) | regs.sr();
      break;
    case IRegister: // addr is register no. in low nibble of instruction
      r = 0x700000 + (regs.rambr << 16) + regs.r[addr];
      break;
    case RegImmediateLong: // addr is immediate address in second two bytes
      r = (pc & 0xff0000) | addr;
      break;
    case RegDirect: // addr is RAM buffer offset in second byte
      r = 0x700000 + (regs.rambr << 16) + (addr << 1);
      break;
    case RegAbsolute: // addr is RAM buffer offset in second two
      r = 0x700000 + (regs.rambr << 16) + addr;
      break;
  }

  return(r & 0xffffff);
}

void SuperFX::disassemble_opcode_ex(SuperFX::Opcode &opcode, uint32 addr, bool alt1, bool alt2) {
  uint8 param[3];

  SNES::debugger.bus_access = true;
  param[0] = superfxbus.read(addr + 0);
  param[1] = superfxbus.read(addr + 1);
  param[2] = superfxbus.read(addr + 2);
  SNES::debugger.bus_access = false;

  SuperFX::OpcodeInfo op = opcode_info(alt1, alt2, param[0]);
  opcode.set(0, op.mode, op.name, param, opcode_length(op.mode) - 1);

  switch (param[0]) {
  case  (0x05):
  case6 (0x98):
    opcode.flags |= Opcode::FLAG_BRA;
    if (param[0] == 0x9b /* jmp to link register */)
      opcode.flags |= Opcode::FLAG_RETURN;
    break;
  case4 (0x06):
  case6 (0x0a):
  case  (0x3c):
    opcode.flags |= Opcode::FLAG_BRA_CONTINUE;
    break;
  case12(0x30):
  case12(0x40):
    opcode.flags |= Opcode::FLAG_INDIRECT;
    break;
  case  (0x3d):
    opcode.flags |= Opcode::FLAG_SET_ALT1;
    break;
  case  (0x3e):
    opcode.flags |= Opcode::FLAG_SET_ALT2;
    break;
  case  (0x3f):
    opcode.flags |= (Opcode::FLAG_SET_ALT1 | Opcode::FLAG_SET_ALT2);
    break;
  }
}

void SuperFX::disassemble_opcode(char *output, uint32 addr, bool track_regs) {
  char t[256] = "";
  *output = 0;
  
  SNES::debugger.bus_access = true;
  uint8 op0 = superfxbus.read(addr + 0);
  uint8 op1 = superfxbus.read(addr + 1);
  uint8 op2 = superfxbus.read(addr + 2);
  SNES::debugger.bus_access = false;

  uint16 disassemble_regs = ((1 << regs.dreg) | (1 << regs.sreg));

  OpcodeInfo op_info = opcode_info(regs.sfr.alt1, regs.sfr.alt2, op0);
  switch (op_info.mode) {
  default:
  case Implied:
  case ImpliedROM:
  case ImpliedRAM:
    break;
  case ImpliedPlot:
    disassemble_regs |= ((1 << 1) | (1 << 2));
    break;
  case ImpliedLongMul:
    disassemble_regs |= ((1 << 4) | (1 << 6));
    break;
  case ImpliedFixedMul:
    disassemble_regs |= (1 << 6);
    break;
  case ImpliedMerge:
    disassemble_regs |= ((1 << 7) | (1 << 8));
    break;
  case ImpliedLoop:
    disassemble_regs |= ((1 << 12) | (1 << 13));
    break;
  case Relative:
    sprintf(t, "$%.4x", (addr + (int8_t)op1 + 2) & 0xffff);
    break;
  case LinkRelative:
    disassemble_regs |= (1 << 11);
    sprintf(t, "$%.4x", (addr + (op0 & 15) + 1) & 0xffff);
    break;
  case Register:
  case RegisterJump:
  case RegisterLJump:
    disassemble_regs |= (1 << (op0 & 15));
    sprintf(t, "r%u", op0 & 15);
    break;
  case IRegister:
    disassemble_regs |= (1 << (op0 & 15));
    sprintf(t, "(r%u)", op0 & 15);
    break;
  case RegImmediate:
    disassemble_regs |= (1 << (op0 & 15));
    sprintf(t, "r%u, #$%.2x", op0 & 15, op1);
    break;
  case RegImmediateLong:
    disassemble_regs |= (1 << (op0 & 15));
    sprintf(t, "r%u, #$%.2x%.2x", op0 & 15, op2, op1);
    break;
  case RegDirect:
    disassemble_regs |= (1 << (op0 & 15));
    sprintf(t, "r%u, $%.3x", op0 & 15, op1 << 1);
    break;  
  case RegAbsolute:
    disassemble_regs |= (1 << (op0 & 15));
    sprintf(t, "r%u, $%.2x%.2x", op0 & 15, op2, op1);
    break;
  case ImmediateShort:
    sprintf(t, "#$0%x", op0 & 15);
    break;
  }
  
  sprintf(output, "%.6x %-5s %-18s", addr, op_info.name, t);
  
  // status register and some flags (TODO: other flags?)
  sprintf(t, "S:%.4x %c%c%c%c ",
      (unsigned) regs.sfr,
      (unsigned) regs.sfr & 2 ? 'Z' : '.',
      (unsigned) regs.sfr & 4 ? 'C' : '.',
      (unsigned) regs.sfr & 8 ? 'N' : '.',
      (unsigned) regs.sfr & 16 ? 'V' : '.');
  strcat(output, t);
  
  // print all current and past used registers
  if (track_regs) {
    for (int i = 0; i < 16; i++) {
      if ((disassemble_regs | disassemble_lastregs) & (1 << i)) {
        sprintf(t, "R%-2u:%.4x ", i, (unsigned) regs.r[i]);
        strcat(output, t);
      }
    }

    disassemble_lastregs = disassemble_regs;
  }
}

SuperFX::OpcodeInfo SuperFX::opcode_info(bool alt1, bool alt2, uint8 opcode) const
{
  bool alt3 = alt1 && alt2;

  switch (opcode) {
  default:
  case  (0x00):
    return { "stop ", Implied };
  case  (0x01):
    return { "nop  ", Implied };
  case  (0x02):
    return { "cache", Implied };
  case  (0x03):
    return { "lsr  ", Implied };
  case  (0x04):
    return { "rol  ", Implied };
  case  (0x05):
    return { "bra  ", Relative };
  case  (0x06):
    return { "bge  ", Relative };
  case  (0x07):
    return { "blt  ", Relative };
  case  (0x08):
    return { "bne  ", Relative };
  case  (0x09):
    return { "beq  ", Relative };
  case  (0x0a):
    return { "bpl  ", Relative };
  case  (0x0b):
    return { "bmi  ", Relative };
  case  (0x0c):
    return { "bcc  ", Relative };
  case  (0x0d):
    return { "bcs  ", Relative };
  case  (0x0e):
    return { "bvc  ", Relative };
  case  (0x0f):
    return { "bvs  ", Relative };
  case16(0x10):
    return { "to   ", Register };
  case16(0x20):
    return { "with ", Register };
  case12(0x30):
    if (alt1) {
      return { "stb  ", IRegister };
    } else {
      return { "stw  ", IRegister };
    }
  case  (0x3c):
    return { "loop ", ImpliedLoop };
  case  (0x3d):
    return { "alt1 ", Implied };
  case  (0x3e):
    return { "alt2 ", Implied };
  case  (0x3f):
    return { "alt3 ", Implied };
  case12(0x40):
    if (alt1) {
      return { "ldb  ", IRegister };
    } else {
      return { "ldw  ", IRegister };
    }
  case  (0x4c):
    if (alt1) {
      return { "rpix ", ImpliedPlot };
    } else {
      return { "plot ", ImpliedPlot };
    }
  case  (0x4d):
    return { "swap ", Implied };
  case  (0x4e):
    if (alt1) {
      return { "cmode", Implied };
    } else {
      return { "color", Implied };
    }
  case  (0x4f):
    return { "not  ", Implied };
  case16(0x50):
    if (alt3) {
      return { "adc  ", ImmediateShort };
    } else if (alt2) {
      return { "add  ", ImmediateShort };
    } else if (alt1) {
      return { "adc  ", Register };
    } else {
      return { "add  ", Register };
    }
  case16(0x60):
    if (alt3) {
      return { "cmp  ", Register };
    } else if (alt2) {
      return { "sub  ", ImmediateShort };
    } else if (alt1) {
      return { "sbc  ", Register };
    } else {
      return { "sub  ", Register };
    }
  case  (0x70):
    return { "merge", ImpliedMerge };
  case15(0x71):
    if (alt3) {
      return { "bic  ", ImmediateShort };
    } else if (alt2) {
      return { "and  ", ImmediateShort };
    } else if (alt1) {
      return { "bic  ", Register };
    } else {
      return { "and  ", Register };
    }
  case16(0x80):
    if (alt3) {
      return { "umult", ImmediateShort };
    } else if (alt2) {
      return { "mult ", ImmediateShort };
    } else if (alt1) {
      return { "umult", Register };
    } else {
      return { "mult ", Register };
    }
  case  (0x90):
    return { "sbk  ", ImpliedRAM };
  case4 (0x91):
    return { "link ", LinkRelative };
  case  (0x95):
    return { "sex  ", Implied };
  case  (0x96):
    if (alt1) {
      return { "div2 ", Implied };
    } else {
      return { "asr  ", Implied };
    }
  case  (0x97):
    return { "ror  ", Implied };
  case6 (0x98):
    if (alt1) {
      return { "ljmp ", RegisterLJump };
    } else {
      return { "jmp  ", RegisterJump };
    }
  case  (0x9e):
    return { "lob  ", Implied };
  case  (0x9f):
    if (alt1) {
      return { "lmult", ImpliedLongMul };
    } else {
      return { "fmult", ImpliedFixedMul };
    }
  case16(0xa0):
    if (alt3) {
      return { "lms  ", Register };
    } else if (alt2) {
      return { "sms  ", RegDirect };
    } else if (alt1) {
      return { "lms  ", RegDirect };
    } else {
      return { "ibt  ", RegImmediate };
    }
  case16(0xb0):
    return { "from ", Register };
  case  (0xc0):
    return { "hib  ", Implied };
  case15(0xc1):
    if (alt3) {
      return { "xor  ", ImmediateShort };
    } else if (alt2) {
      return { "or   ", ImmediateShort };
    } else if (alt1) {
      return { "xor  ", Register };
    } else {
      return { "or   ", Register };
    }
  case15(0xd0):
    return { "inc  ", Register };
  case  (0xdf):
    if (alt3) {
      return { "romb ", Implied };
    } else if (alt2) {
      return { "ramb ", Implied };
    } else {
      return { "getc ", ImpliedROM };
    }
  case15(0xe0):
    return { "dec  ", Register };
  case  (0xef):
    if (alt3) {
      return { "getbs", ImpliedROM };
    } else if (alt2) {
      return { "getbl", ImpliedROM };
    } else if (alt1) {
      return { "getbh", ImpliedROM };
    } else {
      return { "getb ", ImpliedROM };
    }
  case16(0xf0):
    if (alt2) {
      return { "sm   ", RegAbsolute };
    } if (alt1) {
      return { "lm   ", RegAbsolute };
    } else {
      return { "iwt  ", RegImmediateLong };
    }
  }
}

uint8 SuperFX::opcode_length(uint8 offset_type) {
  switch (offset_type) {
  default:
    return 1;
  case Relative:
  case RegImmediate:
  case RegDirect:
    return 2;
  case RegImmediateLong:
  case RegAbsolute:
    return 3;
  }
}

#undef case4
#undef case6
#undef case12
#undef case15
#undef case16

#endif
