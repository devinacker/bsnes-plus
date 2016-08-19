#ifdef SUPERFX_CPP

unsigned SuperFX::cache_access_speed() {
  return regs.clsr.divider;
}

unsigned SuperFX::memory_access_speed() {
  //5 cycles at 21.4 MHz, 3 cycles (6 clocks) at 10.7 MHz
  return regs.clsr.divider + 4;
}

void SuperFX::add_clocks(unsigned clocks) {
  if(regs.romcl) {
    regs.romcl -= min(clocks, regs.romcl);
    if(regs.romcl == 0) {
      regs.sfr.r = 0;
      regs.romdr = superfxbus.read((regs.rombr << 16) + regs.r[14]);
    }
  }

  if(regs.ramcl) {
    regs.ramcl -= min(clocks, regs.ramcl);
    if(regs.ramcl == 0) {
      superfxbus.write(0x700000 + (regs.rambr << 16) + regs.ramar, regs.ramdr);
    }
  }

  step(clocks);
  synchronize_cpu();
}

void SuperFX::rombuffer_sync() {
  if(regs.romcl) add_clocks(regs.romcl);
}

void SuperFX::rombuffer_update() {
  regs.sfr.r = 1;
  regs.romcl = memory_access_speed();
}

uint8 SuperFX::rombuffer_read() {
  rombuffer_sync();
  return regs.romdr;
}

void SuperFX::rambuffer_sync() {
  if(regs.ramcl) add_clocks(regs.ramcl);
}

uint8 SuperFX::rambuffer_read(uint16 addr) {
  rambuffer_sync();
  return superfxbus.read(0x700000 + (regs.rambr << 16) + addr);
}

void SuperFX::rambuffer_write(uint16 addr, uint8 data) {
  rambuffer_sync();
  regs.ramcl = memory_access_speed();
  regs.ramar = addr;
  regs.ramdr = data;
}

void SuperFX::timing_reset() {
  regs.romcl = 0;
  regs.romdr = 0;

  regs.ramcl = 0;
  regs.ramar = 0;
  regs.ramdr = 0;
}

#endif
