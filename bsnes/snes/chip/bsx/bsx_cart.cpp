#ifdef BSX_CPP

BSXCart bsxcart;

void BSXCart::init() {
}

void BSXCart::enable() {
}

void BSXCart::power() {
  reset();
}

void BSXCart::reset() {
  for(unsigned i = 0; i < 16; i++) regs.r[i] = 0x80;
  regs.r[0x01] = 0x00;
  regs.r[0x04] = 0x00;
  regs.r[0x0a] = 0x00;
  regs.r[0x0c] = 0x00;
  regs.r[0x0d] = 0x00;
  regs.dirty = false;
  memcpy(regs.rtemp, regs.r, sizeof(regs.r));

  update_memory_map();
}

void BSXCart::update_memory_map() {
  Memory &cart = cartridge.bsxpack_access();

  //Memory Pack mapping
  if((regs.r[0x02] & 0x80) == 0x00) {
    //LoROM mapping
    bus.map(Bus::MapMode::Linear, 0x00, 0x7d, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0x40, 0x7d, 0x0000, 0x7fff, cart);
    bus.map(Bus::MapMode::Linear, 0x80, 0xff, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0xc0, 0xff, 0x0000, 0x7fff, cart);
  } else {
    //HiROM mapping
    bus.map(Bus::MapMode::Shadow, 0x00, 0x3f, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0x40, 0x7d, 0x0000, 0xffff, cart);
    bus.map(Bus::MapMode::Shadow, 0x80, 0xbf, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0xc0, 0xff, 0x0000, 0xffff, cart);
  }

  //Memory Pack "HOLE" mapping
  if((regs.r[0x02] & 0x80) == 0x00) {
    //LoROM mapping
    if((regs.r[0x09] & 0x80) == 0x80) {
      if((regs.r[0x0b] & 0x80) == 0x00)
        bus.map(Bus::MapMode::Direct, 0x00, 0x1f, 0x8000, 0xffff, memory::memory_unmapped);
      else
        bus.map(Bus::MapMode::Direct, 0x40, 0x4f, 0x8000, 0xffff, memory::memory_unmapped);
    }

    if((regs.r[0x0a] & 0x80) == 0x80) {
      if((regs.r[0x0b] & 0x80) == 0x00)
        bus.map(Bus::MapMode::Direct, 0x80, 0x9f, 0x8000, 0xffff, memory::memory_unmapped);
      else
        bus.map(Bus::MapMode::Direct, 0xc0, 0xdf, 0x8000, 0xffff, memory::memory_unmapped);
    }
  } else {
    //HiROM mapping
    if((regs.r[0x09] & 0x80) == 0x80) {
      if((regs.r[0x0b] & 0x80) == 0x00) {
        bus.map(Bus::MapMode::Direct, 0x00, 0x0f, 0x8000, 0xffff, memory::memory_unmapped);
        bus.map(Bus::MapMode::Direct, 0x40, 0x4f, 0x0000, 0xffff, memory::memory_unmapped);
      } else {
        bus.map(Bus::MapMode::Direct, 0x20, 0x2f, 0x8000, 0xffff, memory::memory_unmapped);
        bus.map(Bus::MapMode::Direct, 0x60, 0x6f, 0x0000, 0xffff, memory::memory_unmapped);
      }
    }

    if((regs.r[0x0a] & 0x80) == 0x80) {
      if((regs.r[0x0b] & 0x80) == 0x00) {
        bus.map(Bus::MapMode::Direct, 0x80, 0x8f, 0x8000, 0xffff, memory::memory_unmapped);
        bus.map(Bus::MapMode::Direct, 0xc0, 0xcf, 0x0000, 0xffff, memory::memory_unmapped);
      } else {
        bus.map(Bus::MapMode::Direct, 0xa0, 0xaf, 0x8000, 0xffff, memory::memory_unmapped);
        bus.map(Bus::MapMode::Direct, 0xe0, 0xef, 0x0000, 0xffff, memory::memory_unmapped);
      }
    }
  }

  //PSRAM mapping
  unsigned psram_shift = (regs.r[0x05] >> 7) | (regs.r[0x06] >> 6);

  if((regs.r[0x02] & 0x80) == 0x00) {
    //LoROM mapping
    if((regs.r[0x03] & 0x80) == 0x80) {
      //Lower Banks (0x00-0x7D)
      bus.map(Bus::MapMode::Linear, 0x00 | (psram_shift * 0x20), 0x0f | (psram_shift * 0x20), 0x8000, 0xffff, memory::bsxpram);
      if(psram_shift >= 2)
        bus.map(Bus::MapMode::Linear, 0x00 | (psram_shift * 0x20), 0x0f | (psram_shift * 0x20), 0x0000, 0x7fff, memory::bsxpram);
      bus.map(Bus::MapMode::Linear, 0x70, 0x7d, 0x0000, 0x7fff, memory::bsxpram);
    }

    if((regs.r[0x04] & 0x80) == 0x80) {
      //Higher Banks (0x80-0xFF)
      bus.map(Bus::MapMode::Linear, 0x80 | (psram_shift * 0x20), 0x8f | (psram_shift * 0x20), 0x8000, 0xffff, memory::bsxpram);
      if(psram_shift >= 2)
        bus.map(Bus::MapMode::Linear, 0x80 | (psram_shift * 0x20), 0x8f | (psram_shift * 0x20), 0x0000, 0x7fff, memory::bsxpram);
      bus.map(Bus::MapMode::Linear, 0xf0, 0xff, 0x0000, 0x7fff, memory::bsxpram);
    }
  } else {
    //HiROM mapping
    if((regs.r[0x03] & 0x80) == 0x80) {
      //Lower Banks (0x00-0x7D)
      bus.map(Bus::MapMode::Linear, 0x40 | (psram_shift * 0x10), 0x47 | (psram_shift * 0x10), 0x0000, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Shadow, 0x20, 0x3f, 0x6000, 0x7fff, memory::bsxpram);
    }

    if((regs.r[0x04] & 0x80) == 0x80) {
      //Higher Banks (0x80-0xFF)
      bus.map(Bus::MapMode::Linear, 0xc0 | (psram_shift * 0x10), 0xc7 | (psram_shift * 0x10), 0x0000, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Shadow, 0xa0, 0xbf, 0x6000, 0x7fff, memory::bsxpram);
    }
  }

  //BS-X ROM mapping
  if(regs.r[0x07] & 0x80) {
    bus.map(Bus::MapMode::Linear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
  }

  if(regs.r[0x08] & 0x80) {
    bus.map(Bus::MapMode::Linear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
  }
}

uint8 BSXCart::read(unsigned addr) {
  uint8 n = (addr >> 16) & 15;
  return regs.rtemp[n];
}

void BSXCart::write(unsigned addr, uint8 data) {
  uint8 n = (addr >> 16) & 15;
  regs.rtemp[n] = data;
	
  // certain games (Treasure Conflix) write these registers way too frequently,
  // causing extremely excessive calls to bus.map() which makes emulation unplayably slow,
  // unless we avoid unnecessarily updating the memory map
  if(n != 0x0e && regs.r[n] != data) {
    regs.dirty = true;
  } else if(n == 0x0e && regs.dirty) {
    memcpy(regs.r, regs.rtemp, sizeof(regs.r));
    update_memory_map();
    regs.dirty = false;
  }
}

BSXCart::BSXCart() {
}

BSXCart::~BSXCart() {
}

#endif

