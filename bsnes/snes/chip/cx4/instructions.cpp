#ifdef CX4_CPP

void Cx4::push() {
  stack[7] = stack[6];
  stack[6] = stack[5];
  stack[5] = stack[4];
  stack[4] = stack[3];
  stack[3] = stack[2];
  stack[2] = stack[1];
  stack[1] = stack[0];
  stack[0] = regs.pc;
}

void Cx4::pull() {
  regs.pc  = stack[0];
  stack[0] = stack[1];
  stack[1] = stack[2];
  stack[2] = stack[3];
  stack[3] = stack[4];
  stack[4] = stack[5];
  stack[5] = stack[6];
  stack[6] = stack[7];
  stack[7] = 0x0000;
}

//Shift-A: math opcodes can shift A register prior to ALU operation
unsigned Cx4::sa() {
  switch(opcode & 0x0300) { default:
  case 0x0000: return regs.a <<  0;
  case 0x0100: return regs.a <<  1;
  case 0x0200: return regs.a <<  8;
  case 0x0300: return regs.a << 16;
  }
}

//Register-or-Immediate: most opcodes can load from a register or immediate
unsigned Cx4::ri() {
  if(opcode & 0x0400) return opcode & 0xff;
  return register_read(opcode & 0xff);
}

//New-PC: determine jump target address; opcode.d9 = long jump flag (1 = yes)
unsigned Cx4::np() {
  if(opcode & 0x0200) return (regs.p << 8) | (opcode & 0xff);
  return (regs.pc & 0xffff00) | (opcode & 0xff);
}

void Cx4::instruction() {
  if((opcode & 0xfc00) == 0x0000) {
    //0000 00.. .... ....
    //nop
  }

  else if((opcode & 0xdd00) == 0x0800) {
    //00.0 10.0 .... ....
    //jump i
    if(opcode & 0x2000) push();
    regs.pc = np();
    change_page();
    add_clocks(2);
  }

  else if((opcode & 0xdd00) == 0x0c00) {
    //00.0 11.0 .... ....
    //jumpeq i
    if(regs.z) {
      if(opcode & 0x2000) push();
      regs.pc = np();
      change_page();
      add_clocks(2);
    }
  }

  else if((opcode & 0xdd00) == 0x1000) {
    //00.1 00.0 .... ....
    //jumpge i
    if(regs.c) {
      if(opcode & 0x2000) push();
      regs.pc = np();
      change_page();
      add_clocks(2);
    }
  }

  else if((opcode & 0xdd00) == 0x1400) {
    //00.1 01.0 .... ....
    //jumpmi i
    if(regs.n) {
      if(opcode & 0x2000) push();
      regs.pc = np();
      change_page();
      add_clocks(2);
    }
  }

  else if((opcode & 0xdd00) == 0x1800) {
    //00.1 10.0 .... ....
    //jumpvs i
    if(regs.v) {
      if(opcode & 0x2000) push();
      regs.pc = np();
      change_page();
      add_clocks(2);
    }
  }

  else if((opcode & 0xfc00) == 0x1c00) {
    //0001 11.. .... ....
    //loop
    if (regs.rwbustime > 1) {
      add_clocks(regs.rwbustime - 1);
    }
  }

  else if((opcode & 0xff00) == 0x2400) {
    //0010 0100 .... ....
    //skipvc/skipvs
    if(regs.v == (opcode & 1)) {
      nextpc();
      add_clocks(1);
    }
  }

  else if((opcode & 0xff00) == 0x2500) {
    //0010 0101 .... ....
    //skiplt/skipge
    if(regs.c == (opcode & 1)) {
      nextpc();
      add_clocks(1);
    }
  }

  else if((opcode & 0xff00) == 0x2600) {
    //0010 0110 .... ....
    //skipne/skipeq
    if(regs.z == (opcode & 1)) {
      nextpc();
      add_clocks(1);
    }
  }

  else if((opcode & 0xff00) == 0x2700) {
    //0010 0111 .... ....
    //skipmi/skippl
    if(regs.n == (opcode & 1)) {
      nextpc();
      add_clocks(1);
    }
  }

  else if((opcode & 0xfc00) == 0x3c00) {
    //0011 11.. .... ....
    //ret
    pull();
    change_page();
    add_clocks(2);
  }

  else if((opcode & 0xfc00) == 0x4000) {
    //0100 00.. .... ....
    //inc
    regs.busaddr++;
  }

  else if((opcode & 0xf800) == 0x4800) {
    //0100 1... .... ....
    //cmpr a<<n,ri
    int result = ri() - sa();
    regs.n = result & 0x800000;
    regs.z = (uint24)result == 0;
    regs.c = result >= 0;
  }

  else if((opcode & 0xf800) == 0x5000) {
    //0101 0... .... ....
    //cmp a<<n,ri
    int result = sa() - ri();
    regs.n = result & 0x800000;
    regs.z = (uint24)result == 0;
    regs.c = result >= 0;
  }

  else if((opcode & 0xfb00) == 0x5900) {
    //0101 1.01 .... ....
    //sxb
    regs.a = (int8)ri();
  }

  else if((opcode & 0xfb00) == 0x5a00) {
    //0101 1.10 .... ....
    //sxw
    regs.a = (int16)ri();
  }

  else if((opcode & 0xfb00) == 0x6000) {
    //0110 0.00 .... ....
    //ld a,ri
    regs.a = ri();
  }

  else if((opcode & 0xfb00) == 0x6100) {
    //0110 0.01 .... ....
    //rdbus r
    register_read(opcode & 0xff);
  }

  else if((opcode & 0xfb00) == 0x6300) {
    //0110 0.11 .... ....
    //ld p,ri
    regs.p = ri();
  }

  else if((opcode & 0xfb00) == 0x6800) {
    //0110 1.00 .... ....
    //rdraml
    uint24 target = ri() + (opcode & 0x0400 ? regs.ramaddr : (uint24)0);
    if(target < 0xc00) {
      regs.ramdata = (regs.ramdata & 0xffff00) | (dataRAM[target] <<  0);
      regs.mdr = regs.ramdata;
    }
  }

  else if((opcode & 0xfb00) == 0x6900) {
    //0110 1.01 .... ....
    //rdramh
    uint24 target = ri() + (opcode & 0x0400 ? regs.ramaddr : (uint24)0);
    if(target < 0xc00) {
      regs.ramdata = (regs.ramdata & 0xff00ff) | (dataRAM[target] <<  8);
      regs.mdr = regs.ramdata;
    }
  }

  else if((opcode & 0xfb00) == 0x6a00) {
    //0110 1.10 .... ....
    //rdramb
    uint24 target = ri() + (opcode & 0x0400 ? regs.ramaddr : (uint24)0);
    if(target < 0xc00) {
      regs.ramdata = (regs.ramdata & 0x00ffff) | (dataRAM[target] << 16);
      regs.mdr = regs.ramdata;
    }
  }

  else if((opcode & 0xffff) == 0x7000) {
    //0111 0000 0000 0000
    //rdrom
    regs.romdata = dataROM[regs.a & 0x3ff];
  }

  else if((opcode & 0xff00) == 0x7c00) {
    //0111 1100 .... ....
    //ld pl,i
    regs.p = (regs.p & 0xff00) | ((opcode & 0xff) << 0);
  }

  else if((opcode & 0xff00) == 0x7d00) {
    //0111 1101 .... ....
    //ld ph,i
    regs.p = (regs.p & 0x00ff) | ((opcode & 0xff) << 8);
  }

  else if((opcode & 0xf800) == 0x8000) {
    //1000 0... .... ....
    //add a<<n,ri
    int x = sa();
    int y = ri();
    int result = x + y;
    regs.a = result;
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
    regs.c = result > 0xffffff;
    regs.v = ~(x & y) & (x ^ result) & (1 << 23);
  }

  else if((opcode & 0xf800) == 0x8800) {
    //1000 1... .... ....
    //subr a<<n,ri
    int x = ri();
    int y = sa();
    int result = x - y;
    regs.a = result;
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
    regs.c = result >= 0;
    regs.v = ~(x & y) & (x ^ result) & (1 << 23);
  }

  else if((opcode & 0xf800) == 0x9000) {
    //1001 0... .... ....
    //sub a<<n,ri
    int x = sa();
    int y = ri();
    int result = x - y;
    regs.a = result;
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
    regs.c = result >= 0;
    regs.v = ~(x & y) & (x ^ result) & (1 << 23);
  }

  else if((opcode & 0xfb00) == 0x9800) {
    //1001 1.00 .... ....
    //mul a,ri
    int64 x = (int24)regs.a;
    int64 y = (int24)ri();
    x *= y;
    regs.accl = x >>  0ull;
    regs.acch = x >> 24ull;
    regs.n = regs.acch & 0x800000;
    regs.z = x == 0;
  }

  else if((opcode & 0xf800) == 0xa800) {
    //1010 1... .... ....
    //xor a<<n,ri
    regs.a = sa() ^ ri();
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xf800) == 0xb000) {
    //1011 0... .... ....
    //and a<<n,ri
    regs.a = sa() & ri();
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xf800) == 0xb800) {
    //1011 1... .... ....
    //or a<<n,ri
    regs.a = sa() | ri();
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xfb00) == 0xc000) {
    //1100 0.00 .... ....
    //shr a,ri
    regs.a = regs.a >> ri();
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xfb00) == 0xc800) {
    //1100 1.00 .... ....
    //asr a,ri
    regs.a = (int24)regs.a >> ri();
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xfb00) == 0xd000) {
    //1101 0.00 .... ....
    //ror a,ri
    uint24 length = ri();
    regs.a = (regs.a >> length) | (regs.a << (24 - length));
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xfb00) == 0xd800) {
    //1101 1.00 .... ....
    //shl a,ri
    regs.a = regs.a << ri();
    regs.n = regs.a & 0x800000;
    regs.z = regs.a == 0;
  }

  else if((opcode & 0xff00) == 0xe000) {
    //1110 0000 .... ....
    //st r,a
    register_write(opcode & 0xff, regs.a);
  }
  
  else if((opcode & 0xff00) == 0xe1000) {
    //1110 0001 .... ....
    //wrbus r
    register_write(opcode & 0xff, regs.busdata);
  }

  else if((opcode & 0xfb00) == 0xe800) {
    //1110 1.00 .... ....
    //wrraml
    uint24 target = ri() + (opcode & 0x0400 ? regs.ramaddr : (uint24)0);
    if(target < 0xc00) {
      dataRAM[target] = regs.ramdata >>  0;
      regs.mdr = regs.ramdata;
    }
  }

  else if((opcode & 0xfb00) == 0xe900) {
    //1110 1.01 .... ....
    //wrramh
    uint24 target = ri() + (opcode & 0x0400 ? regs.ramaddr : (uint24)0);
    if(target < 0xc00) {
      dataRAM[target] = regs.ramdata >>  8;
      regs.mdr = regs.ramdata;
    }
  }

  else if((opcode & 0xfb00) == 0xea00) {
    //1110 1.10 .... ....
    //wrramb
    uint24 target = ri() + (opcode & 0x0400 ? regs.ramaddr : (uint24)0);
    if(target < 0xc00) {
      dataRAM[target] = regs.ramdata >> 16;
      regs.mdr = regs.ramdata;
    }
  }

  else if((opcode & 0xfc00) == 0xf000) {
    //1111 00.. .... ....
    //swap a,r
    uint24 source = register_read(opcode & 0xff);
    uint24 target = regs.a;
    regs.a = source;
    register_write(opcode & 0xff, target);
  }

  else if((opcode & 0xfc00) == 0xf800) {
    //1111 10.. .... ....
    //clear
    regs.a = 0;
    regs.p = 0;
    regs.ramdata = 0;
    regs.ramaddr = 0;
  }

  else if((opcode & 0xfc00) == 0xfc00) {
    //1111 11.. .... ....
    //halt
    regs.halt = true;
  }

  else {
    // unknown opcode
    regs.halt = true;
  }

  add_clocks(1);
}

#endif
