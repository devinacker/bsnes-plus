#ifdef SUPERFX_CPP

void SuperFX::disassemble_opcode(char *output, uint32 addr, bool track_regs) {
  char t[256] = "";

  *output = 0;
  
  int temp_regs = disassemble_regs;

  SNES::debugger.bus_access = true;
  if(!regs.sfr.alt2) {
    if(!regs.sfr.alt1) {
      disassemble_alt0(t, addr);
    } else {
      disassemble_alt1(t, addr);
    }
  } else {
    if(!regs.sfr.alt1) {
      disassemble_alt2(t, addr);
    } else {
      disassemble_alt3(t, addr);
    }
  }
  SNES::debugger.bus_access = false;
  
  sprintf(output, "%.6x %s", addr, t);

  unsigned length = strlen(output);
  while(length++ < 25) strcat(output, " ");
  
  // status register and some flags (TODO: other flags?)
  sprintf(t, "S:%.4x %c%c%c%c ",
      (unsigned) regs.sfr,
      (unsigned) regs.sfr & 2 ? 'Z' : 'z',
      (unsigned) regs.sfr & 4 ? 'C' : 'c',
      (unsigned) regs.sfr & 8 ? 'N' : 'n',
      (unsigned) regs.sfr & 16 ? 'V' : 'v');
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
    disassemble_regs = 0;
  } else {
    disassemble_regs = temp_regs;
  }
}

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

#define op0 superfxbus.read(addr + 0)
#define op1 superfxbus.read(addr + 1)
#define op2 superfxbus.read(addr + 2)
#define bdest (addr + (int8_t)op1 + 2) & 0xffff

#define R(n) disassemble_regs |= (1 << n)
#define Ri disassemble_regs |= (1 << (op0 & 15))
#define Rs disassemble_regs |= (1 << regs.sreg)
#define Rd disassemble_regs |= (1 << regs.dreg)
#define Rsd Rs; Rd

void SuperFX::disassemble_alt0(char *output, uint32 addr) {
  char t[256] = "";
  switch(op0) {
    case  (0x00): sprintf(t, "stop  "); break;
    case  (0x01): sprintf(t, "nop   "); break;
    case  (0x02): sprintf(t, "cache "); break;
    case  (0x03): sprintf(t, "lsr   "); Rsd; break;
    case  (0x04): sprintf(t, "rol   "); Rsd; break;
    case  (0x05): sprintf(t, "bra   $%.4x", bdest); break;
    case  (0x06): sprintf(t, "bge   $%.4x", bdest); break;
    case  (0x07): sprintf(t, "blt   $%.4x", bdest); break;
    case  (0x08): sprintf(t, "bne   $%.4x", bdest); break;
    case  (0x09): sprintf(t, "beq   $%.4x", bdest); break;
    case  (0x0a): sprintf(t, "bpl   $%.4x", bdest); break;
    case  (0x0b): sprintf(t, "bmi   $%.4x", bdest); break;
    case  (0x0c): sprintf(t, "bcc   $%.4x", bdest); break;
    case  (0x0d): sprintf(t, "bcs   $%.4x", bdest); break;
    case  (0x0e): sprintf(t, "bvc   $%.4x", bdest); break;
    case  (0x0f): sprintf(t, "bvs   $%.4x", bdest); break;
    case16(0x10): sprintf(t, "to    r%u", op0 & 15); Ri; break;
    case16(0x20): sprintf(t, "with  r%u", op0 & 15); Ri; break;
    case12(0x30): sprintf(t, "stw   (r%u)", op0 & 15); Ri; break;
    case  (0x3c): sprintf(t, "loop  "); R(12); R(13); break;
    case  (0x3d): sprintf(t, "alt1  "); break;
    case  (0x3e): sprintf(t, "alt2  "); break;
    case  (0x3f): sprintf(t, "alt3  "); break;
    case12(0x40): sprintf(t, "ldw   (r%u)", op0 & 15); Ri; break;
    case  (0x4c): sprintf(t, "plot  "); R(1); R(2); break;
    case  (0x4d): sprintf(t, "swap  "); Rsd; break;
    case  (0x4e): sprintf(t, "color "); Rs; break;
    case  (0x4f): sprintf(t, "not   "); Rsd; break;
    case16(0x50): sprintf(t, "add   r%u", op0 & 15); Ri; Rsd; break;
    case16(0x60): sprintf(t, "sub   r%u", op0 & 15); Ri; Rsd; break;
    case  (0x70): sprintf(t, "merge "); Rd; R(7); R(8); break;
    case15(0x71): sprintf(t, "and   r%u", op0 & 15); Ri; Rsd; break;
    case16(0x80): sprintf(t, "mult  r%u", op0 & 15); Ri; Rsd; break;
    case  (0x90): sprintf(t, "sbk   "); Rs; break;
    case4 (0x91): sprintf(t, "link  #%u", op0 & 15); R(11); break;
    case  (0x95): sprintf(t, "sex   "); Rsd; break;
    case  (0x96): sprintf(t, "asr   "); Rsd; break;
    case  (0x97): sprintf(t, "ror   "); Rsd; break;
    case6 (0x98): sprintf(t, "jmp   r%u", op0 & 15); Ri; break;
    case  (0x9e): sprintf(t, "lob   "); Rsd; break;
    case  (0x9f): sprintf(t, "fmult "); Rsd; R(6); break;
    case16(0xa0): sprintf(t, "ibt   r%u,#$%.2x", op0 & 15, op1); Ri; break;
    case16(0xb0): sprintf(t, "from  r%u", op0 & 15); Ri; break;
    case  (0xc0): sprintf(t, "hib   "); Rsd; break;
    case15(0xc1): sprintf(t, "or    r%u", op0 & 15); Ri; Rsd; break;
    case15(0xd0): sprintf(t, "inc   r%u", op0 & 15); Ri; break;
    case  (0xdf): sprintf(t, "getc  "); break;
    case15(0xe0): sprintf(t, "dec   r%u", op0 & 15); Ri; break;
    case  (0xef): sprintf(t, "getb  "); Rd; break;
    case16(0xf0): sprintf(t, "iwt   r%u,#$%.2x%.2x", op0 & 15, op2, op1); Ri; break;
  }
  strcat(output, t);
}

void SuperFX::disassemble_alt1(char *output, uint32 addr) {
  char t[256] = "";
  switch(op0) {
    case  (0x00): sprintf(t, "stop  "); break;
    case  (0x01): sprintf(t, "nop   "); break;
    case  (0x02): sprintf(t, "cache "); break;
    case  (0x03): sprintf(t, "lsr   "); Rsd; break;
    case  (0x04): sprintf(t, "rol   "); Rsd; break;
    case  (0x05): sprintf(t, "bra   $%.4x", bdest); break;
    case  (0x06): sprintf(t, "bge   $%.4x", bdest); break;
    case  (0x07): sprintf(t, "blt   $%.4x", bdest); break;
    case  (0x08): sprintf(t, "bne   $%.4x", bdest); break;
    case  (0x09): sprintf(t, "beq   $%.4x", bdest); break;
    case  (0x0a): sprintf(t, "bpl   $%.4x", bdest); break;
    case  (0x0b): sprintf(t, "bmi   $%.4x", bdest); break;
    case  (0x0c): sprintf(t, "bcc   $%.4x", bdest); break;
    case  (0x0d): sprintf(t, "bcs   $%.4x", bdest); break;
    case  (0x0e): sprintf(t, "bvc   $%.4x", bdest); break;
    case  (0x0f): sprintf(t, "bvs   $%.4x", bdest); break;
    case16(0x10): sprintf(t, "to    r%u", op0 & 15); Ri; break;
    case16(0x20): sprintf(t, "with  r%u", op0 & 15); Ri; break;
    case12(0x30): sprintf(t, "stb   (r%u)", op0 & 15); Ri; break;
    case  (0x3c): sprintf(t, "loop  "); R(12); R(13); break;
    case  (0x3d): sprintf(t, "alt1  "); break;
    case  (0x3e): sprintf(t, "alt2  "); break;
    case  (0x3f): sprintf(t, "alt3  "); break;
    case12(0x40): sprintf(t, "ldb   (r%u)", op0 & 15); Ri; break;
    case  (0x4c): sprintf(t, "rpix  "); R(1); R(2); break;
    case  (0x4d): sprintf(t, "swap  "); Rsd; break;
    case  (0x4e): sprintf(t, "cmode "); Rs; break;
    case  (0x4f): sprintf(t, "not   "); Rsd; break;
    case16(0x50): sprintf(t, "adc   r%u", op0 & 15); Ri; Rsd; break;
    case16(0x60): sprintf(t, "sbc   r%u", op0 & 15); Ri; Rsd; break;
    case  (0x70): sprintf(t, "merge "); Rd; R(7); R(8); break;
    case15(0x71): sprintf(t, "bic   r%u", op0 & 15); Ri; Rsd; break;
    case16(0x80): sprintf(t, "umult r%u", op0 & 15); Ri; Rsd; break;
    case  (0x90): sprintf(t, "sbk   "); Rs; break;
    case4 (0x91): sprintf(t, "link  #%u", op0 & 15); R(11); break;
    case  (0x95): sprintf(t, "sex   "); Rsd; break;
    case  (0x96): sprintf(t, "div2  "); Rsd; break;
    case  (0x97): sprintf(t, "ror   "); Rsd; break;
    case6 (0x98): sprintf(t, "ljmp  r%u", op0 & 15); Ri; Rs; break;
    case  (0x9e): sprintf(t, "lob   "); Rsd; break;
    case  (0x9f): sprintf(t, "lmult "); Rsd; R(4); R(6); break;
    case16(0xa0): sprintf(t, "lms   r%u,(#$%.4x)", op0 & 15, op1 << 1); Ri; break;
    case16(0xb0): sprintf(t, "from  r%u", op0 & 15); Ri; break;
    case  (0xc0): sprintf(t, "hib   "); Rsd; break;
    case15(0xc1): sprintf(t, "xor   r%u", op0 & 15); Ri; Rsd; break;
    case15(0xd0): sprintf(t, "inc   r%u", op0 & 15); Ri; break;
    case  (0xdf): sprintf(t, "getc  "); break;
    case15(0xe0): sprintf(t, "dec   r%u", op0 & 15); Ri; break;
    case  (0xef): sprintf(t, "getbh "); Rsd; break;
    case16(0xf0): sprintf(t, "lm    r%u", op0 & 15); Ri; break;
  }
  strcat(output, t);
}

void SuperFX::disassemble_alt2(char *output, uint32 addr) {
  char t[256] = "";
  switch(op0) {
    case  (0x00): sprintf(t, "stop  "); break;
    case  (0x01): sprintf(t, "nop   "); break;
    case  (0x02): sprintf(t, "cache "); break;
    case  (0x03): sprintf(t, "lsr   "); Rsd; break;
    case  (0x04): sprintf(t, "rol   "); Rsd; break;
    case  (0x05): sprintf(t, "bra   $%.4x", bdest); break;
    case  (0x06): sprintf(t, "bge   $%.4x", bdest); break;
    case  (0x07): sprintf(t, "blt   $%.4x", bdest); break;
    case  (0x08): sprintf(t, "bne   $%.4x", bdest); break;
    case  (0x09): sprintf(t, "beq   $%.4x", bdest); break;
    case  (0x0a): sprintf(t, "bpl   $%.4x", bdest); break;
    case  (0x0b): sprintf(t, "bmi   $%.4x", bdest); break;
    case  (0x0c): sprintf(t, "bcc   $%.4x", bdest); break;
    case  (0x0d): sprintf(t, "bcs   $%.4x", bdest); break;
    case  (0x0e): sprintf(t, "bvc   $%.4x", bdest); break;
    case  (0x0f): sprintf(t, "bvs   $%.4x", bdest); break;
    case16(0x10): sprintf(t, "to    r%u", op0 & 15); Ri; break;
    case16(0x20): sprintf(t, "with  r%u", op0 & 15); Ri; break;
    case12(0x30): sprintf(t, "stw   (r%u)", op0 & 15); Ri; break;
    case  (0x3c): sprintf(t, "loop  "); R(12); R(13); break;
    case  (0x3d): sprintf(t, "alt1  "); break;
    case  (0x3e): sprintf(t, "alt2  "); break;
    case  (0x3f): sprintf(t, "alt3  "); break;
    case12(0x40): sprintf(t, "ldw   (r%u)", op0 & 15); Ri; break;
    case  (0x4c): sprintf(t, "plot  "); R(1); R(2); break;
    case  (0x4d): sprintf(t, "swap  "); Rsd; break;
    case  (0x4e): sprintf(t, "color "); Rs; break;
    case  (0x4f): sprintf(t, "not   "); Rsd; break;
    case16(0x50): sprintf(t, "add   #%u", op0 & 15); Ri; Rsd; break;
    case16(0x60): sprintf(t, "sub   #%u", op0 & 15); Ri; Rsd; break;
    case  (0x70): sprintf(t, "merge "); Rd; R(7); R(8); break;
    case15(0x71): sprintf(t, "and   #%u", op0 & 15); Ri; Rsd; break;
    case16(0x80): sprintf(t, "mult  #%u", op0 & 15); Ri; Rsd; break;
    case  (0x90): sprintf(t, "sbk   "); Rs; break;
    case4 (0x91): sprintf(t, "link  #%u", op0 & 15); R(11); break;
    case  (0x95): sprintf(t, "sex   "); Rsd; break;
    case  (0x96): sprintf(t, "asr   "); Rsd; break;
    case  (0x97): sprintf(t, "ror   "); Rsd; break;
    case6 (0x98): sprintf(t, "jmp   r%u", op0 & 15); Ri; break;
    case  (0x9e): sprintf(t, "lob   "); Rsd; break;
    case  (0x9f): sprintf(t, "fmult "); Rsd; R(6); break;
    case16(0xa0): sprintf(t, "sms   r%u,(#$%.4x)", op0 & 15, op1 << 1); Ri; break;
    case16(0xb0): sprintf(t, "from  r%u", op0 & 15); Ri; break;
    case  (0xc0): sprintf(t, "hib   "); Rsd; break;
    case15(0xc1): sprintf(t, "or    #%u", op0 & 15); Ri; Rsd; break;
    case15(0xd0): sprintf(t, "inc   r%u", op0 & 15); Ri; break;
    case  (0xdf): sprintf(t, "ramb  "); Rs; break;
    case15(0xe0): sprintf(t, "dec   r%u", op0 & 15); Ri; break;
    case  (0xef): sprintf(t, "getbl "); Rsd; break;
    case16(0xf0): sprintf(t, "sm    r%u", op0 & 15); break;
  }
  strcat(output, t);
}

void SuperFX::disassemble_alt3(char *output, uint32 addr) {
  char t[256] = "";
  switch(op0) {
    case  (0x00): sprintf(t, "stop  "); break;
    case  (0x01): sprintf(t, "nop   "); break;
    case  (0x02): sprintf(t, "cache "); break;
    case  (0x03): sprintf(t, "lsr   "); Rsd; break;
    case  (0x04): sprintf(t, "rol   "); Rsd; break;
    case  (0x05): sprintf(t, "bra   $%.4x", bdest); break;
    case  (0x06): sprintf(t, "bge   $%.4x", bdest); break;
    case  (0x07): sprintf(t, "blt   $%.4x", bdest); break;
    case  (0x08): sprintf(t, "bne   $%.4x", bdest); break;
    case  (0x09): sprintf(t, "beq   $%.4x", bdest); break;
    case  (0x0a): sprintf(t, "bpl   $%.4x", bdest); break;
    case  (0x0b): sprintf(t, "bmi   $%.4x", bdest); break;
    case  (0x0c): sprintf(t, "bcc   $%.4x", bdest); break;
    case  (0x0d): sprintf(t, "bcs   $%.4x", bdest); break;
    case  (0x0e): sprintf(t, "bvc   $%.4x", bdest); break;
    case  (0x0f): sprintf(t, "bvs   $%.4x", bdest); break;
    case16(0x10): sprintf(t, "to    r%u", op0 & 15); Ri; break;
    case16(0x20): sprintf(t, "with  r%u", op0 & 15); Ri; break;
    case12(0x30): sprintf(t, "stb   (r%u)", op0 & 15); Ri; break;
    case  (0x3c): sprintf(t, "loop  "); R(12); R(13); break;
    case  (0x3d): sprintf(t, "alt1  "); break;
    case  (0x3e): sprintf(t, "alt2  "); break;
    case  (0x3f): sprintf(t, "alt3  "); break;
    case12(0x40): sprintf(t, "ldb   (r%u)", op0 & 15); Ri; break;
    case  (0x4c): sprintf(t, "rpix  "); Rd; R(1); R(2); break;
    case  (0x4d): sprintf(t, "swap  "); Rsd; break;
    case  (0x4e): sprintf(t, "cmode "); Rs; break;
    case  (0x4f): sprintf(t, "not   "); Rsd; break;
    case16(0x50): sprintf(t, "adc   #%u", op0 & 15); Ri; Rsd; break;
    case16(0x60): sprintf(t, "cmp   r%u", op0 & 15); Ri; Rs; break;
    case  (0x70): sprintf(t, "merge "); Rd; R(7); R(8); break;
    case15(0x71): sprintf(t, "bic   #%u", op0 & 15); Ri; Rsd; break;
    case16(0x80): sprintf(t, "umult #%u", op0 & 15); Ri; Rsd; break;
    case  (0x90): sprintf(t, "sbk   "); Rs; break;
    case4 (0x91): sprintf(t, "link  #%u", op0 & 15); R(11); break;
    case  (0x95): sprintf(t, "sex   "); Rsd; break;
    case  (0x96): sprintf(t, "div2  "); Rsd; break;
    case  (0x97): sprintf(t, "ror   "); Rsd; break;
    case6 (0x98): sprintf(t, "ljmp  r%u", op0 & 15); Ri; Rs; break;
    case  (0x9e): sprintf(t, "lob   "); Rsd; break;
    case  (0x9f): sprintf(t, "lmult "); Rsd; R(4); R(6); break;
    case16(0xa0): sprintf(t, "lms   r%u", op0 & 15); Ri; break;
    case16(0xb0): sprintf(t, "from  r%u", op0 & 15); Ri; break;
    case  (0xc0): sprintf(t, "hib   "); Rsd; break;
    case15(0xc1): sprintf(t, "xor   #%u", op0 & 15); Ri; Rsd; break;
    case15(0xd0): sprintf(t, "inc   r%u", op0 & 15); Ri; break;
    case  (0xdf): sprintf(t, "romb  "); Rs; break;
    case15(0xe0): sprintf(t, "dec   r%u", op0 & 15); Ri; break;
    case  (0xef): sprintf(t, "getbs "); Rd; break;
    case16(0xf0): sprintf(t, "lm    r%u", op0 & 15); Ri; break;
  }
  strcat(output, t);
}

#undef case4
#undef case6
#undef case12
#undef case15
#undef case16
#undef op0
#undef op1
#undef op2
#undef bdest
#undef R
#undef Ri
#undef Rs
#undef Rd
#undef Rsd

#endif
