#ifdef SUPERFX_CPP

uint8 SuperFX::load_cache(unsigned dp, unsigned target) {
  unsigned sp = (regs.pbr << 16) + (uint16)(regs.cbr + dp);
  //$[00-5f]:[0000-ffff] ROM
  //$[60-7f]:[0000-ffff] RAM
  (regs.pbr <= 0x5f) ? rombuffer_sync() : rambuffer_sync();
  do {
    add_clocks(memory_access_speed());
    cache.buffer[dp++] = superfxbus.read(sp++);
  } while(dp <= target);
  if(cache.partial != cache.Flushed) {
    cache.partial = dp;
    if((target & 15) == 15) cache.valid[target >> 4] = true;
  }
  return cache.buffer[target];
}

uint8 SuperFX::op_read(uint16 addr) {
  unsigned offset = addr - regs.cbr;
  if(offset < 512) {
    if(cache.valid[offset >> 4] == false) {
      cache.partial = (offset & ~0xf) | (cache.partial & 0xf);
      return load_cache(cache.partial, offset);
    } else {
      add_clocks(cache_access_speed());
      return cache.buffer[offset];
    }
  }

  (regs.pbr <= 0x5f) ? rombuffer_sync() : rambuffer_sync();
  add_clocks(memory_access_speed());
  return superfxbus.read((regs.pbr << 16) + addr);
}

uint8 SuperFX::peekpipe() {
  uint8 result = regs.pipeline;
  regs.pipeline = op_read(regs.r[15]);
  return result;
}

uint8 SuperFX::pipe() {
  uint8 result = regs.pipeline;
  regs.pipeline = op_read(++regs.r[15]);
  return result;
}

void SuperFX::cache_finish() {
  if(cache.partial & 15) {
    load_cache(cache.partial, cache.partial | 15);
  }
}

void SuperFX::cache_flush() {
  cache.partial = cache.Flushed;
  for(unsigned n = 0; n < 32; n++) cache.valid[n] = false;
}

uint8 SuperFX::cache_mmio_read(uint16 addr) {
  addr = (addr + regs.cbr) & 511;
  return cache.buffer[addr];
}

void SuperFX::cache_mmio_write(uint16 addr, uint8 data) {
  addr = (addr + regs.cbr) & 511;
  cache.buffer[addr] = data;
  if((addr & 15) == 15) cache.valid[addr >> 4] = true;
}

void SuperFX::memory_reset() {
  cache_flush();
  for(unsigned n = 0; n < 512; n++) cache.buffer[n] = 0x00;
  for(unsigned n = 0; n < 2; n++) {
    pixelcache[n].offset = ~0;
    pixelcache[n].bitpend = 0x00;
  }
}

#endif
