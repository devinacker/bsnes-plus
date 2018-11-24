#ifdef CX4_CPP

uint24 Cx4::register_read(uint8 addr) {
  uint24 data = 0x000000;

  switch(addr & 0x7f) {
  case 0x00: data = regs.a; break;
  case 0x01: data = regs.acch; break;
  case 0x02: data = regs.accl; break;
  case 0x03: data = regs.busdata; break;
  case 0x08: data = regs.romdata; break;
  case 0x0c: data = regs.ramdata; break;
  case 0x13: data = regs.busaddr; break;
  case 0x1c: data = regs.ramaddr; break;
  case 0x20: data = regs.pc & 0xff; // already incremented to next instruction before access
  case 0x28: data = regs.p; break;
  case 0x2e: case 0x2f:
    regs.rwbusaddr = regs.busaddr;
    regs.rwbustime = ((addr & 1) ? mmio.ramSpeed : mmio.romSpeed) + 1; //includes current cycle
    regs.writebus = false;
    break;
  case 0x50: data = 0x000000; break;
  case 0x51: data = 0xffffff; break;
  case 0x52: data = 0x00ff00; break;
  case 0x53: data = 0xff0000; break;
  case 0x54: data = 0x00ffff; break;
  case 0x55: data = 0xffff00; break;
  case 0x56: data = 0x800000; break;
  case 0x57: data = 0x7fffff; break;
  case 0x58: data = 0x008000; break;
  case 0x59: data = 0x007fff; break;
  case 0x5a: data = 0xff7fff; break;
  case 0x5b: data = 0xffff7f; break;
  case 0x5c: data = 0x010000; break;
  case 0x5d: data = 0xfeffff; break;
  case 0x5e: data = 0x000100; break;
  case 0x5f: data = 0x00feff; break;
  case 0x60: case 0x70: data = regs.gpr[ 0]; break;
  case 0x61: case 0x71: data = regs.gpr[ 1]; break;
  case 0x62: case 0x72: data = regs.gpr[ 2]; break;
  case 0x63: case 0x73: data = regs.gpr[ 3]; break;
  case 0x64: case 0x74: data = regs.gpr[ 4]; break;
  case 0x65: case 0x75: data = regs.gpr[ 5]; break;
  case 0x66: case 0x76: data = regs.gpr[ 6]; break;
  case 0x67: case 0x77: data = regs.gpr[ 7]; break;
  case 0x68: case 0x78: data = regs.gpr[ 8]; break;
  case 0x69: case 0x79: data = regs.gpr[ 9]; break;
  case 0x6a: case 0x7a: data = regs.gpr[10]; break;
  case 0x6b: case 0x7b: data = regs.gpr[11]; break;
  case 0x6c: case 0x7c: data = regs.gpr[12]; break;
  case 0x6d: case 0x7d: data = regs.gpr[13]; break;
  case 0x6e: case 0x7e: data = regs.gpr[14]; break;
  case 0x6f: case 0x7f: data = regs.gpr[15]; break;
  }
  return regs.mdr = data;
}

void Cx4::register_write(uint8 addr, uint24 data) {
  regs.mdr = data;
  
  switch(addr & 0x7f) {
  case 0x00: regs.a = data; return;
  case 0x01: regs.acch = data; return;
  case 0x02: regs.accl = data; return;
  case 0x03: regs.busdata = data; return;
  case 0x08: regs.romdata = data; return;
  case 0x0c: regs.ramdata = data; return;
  case 0x13: regs.busaddr = data; return;
  case 0x1c: regs.ramaddr = data; return;
  case 0x2e: case 0x2f:     
    regs.rwbusaddr = regs.busaddr;
    regs.rwbustime = ((addr & 1) ? mmio.ramSpeed : mmio.romSpeed) + 1; //includes current cycle
    regs.writebus = true;
    regs.writebusdata = data;
    return;
  case 0x60: case 0x70: regs.gpr[ 0] = data; return;
  case 0x61: case 0x71: regs.gpr[ 1] = data; return;
  case 0x62: case 0x72: regs.gpr[ 2] = data; return;
  case 0x63: case 0x73: regs.gpr[ 3] = data; return;
  case 0x64: case 0x74: regs.gpr[ 4] = data; return;
  case 0x65: case 0x75: regs.gpr[ 5] = data; return;
  case 0x66: case 0x76: regs.gpr[ 6] = data; return;
  case 0x67: case 0x77: regs.gpr[ 7] = data; return;
  case 0x68: case 0x78: regs.gpr[ 8] = data; return;
  case 0x69: case 0x79: regs.gpr[ 9] = data; return;
  case 0x6a: case 0x7a: regs.gpr[10] = data; return;
  case 0x6b: case 0x7b: regs.gpr[11] = data; return;
  case 0x6c: case 0x7c: regs.gpr[12] = data; return;
  case 0x6d: case 0x7d: regs.gpr[13] = data; return;
  case 0x6e: case 0x7e: regs.gpr[14] = data; return;
  case 0x6f: case 0x7f: regs.gpr[15] = data; return;
  }
}

#endif
