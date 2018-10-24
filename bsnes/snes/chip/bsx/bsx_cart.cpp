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
  for(unsigned i = 0; i < 16; i++) regs.r[i] = true;
  regs.irq     = false;
  regs.irq_en  = false;
  regs.r[0x04] = false;
  regs.r[0x0a] = false;
  regs.r[0x0c] = false;
  regs.r[0x0d] = false;
  regs.dirty   = false;
  memcpy(regs.rtemp, regs.r, sizeof(regs.r));

  update_memory_map();
}

void BSXCart::update_memory_map() {
  Memory &cart = cartridge.bsxpack_access();

  //Memory Pack mapping
  if(!regs.r[0x02]) {
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

  //Memory Pack "HOLE" mapping (missing external memory)
  unsigned ext_shift = (regs.r[0x0b] << 5);
  
  if(!regs.r[0x02]) {
    //LoROM mapping
    ext_shift <<= 1;
    unsigned ext_base = (ext_shift & 0x40) ? 0x0000 : 0x8000;
    
    if(regs.r[0x09]) {
      bus.map(Bus::MapMode::Direct, 0x00 | ext_shift, 0x1f | ext_shift, ext_base, 0xffff, memory::memory_unmapped);
    }
    
    if(regs.r[0x0a]) {
      bus.map(Bus::MapMode::Direct, 0x80 | ext_shift, 0x9f | ext_shift, ext_base, 0xffff, memory::memory_unmapped);
    }
  } else {
    //HiROM mapping
    if(regs.r[0x09]) {
      bus.map(Bus::MapMode::Direct, 0x00 | ext_shift, 0x0f | ext_shift, 0x8000, 0xffff, memory::memory_unmapped);
      bus.map(Bus::MapMode::Direct, 0x40 | ext_shift, 0x4f | ext_shift, 0x0000, 0xffff, memory::memory_unmapped);
    }

    if(regs.r[0x0a]) {
      bus.map(Bus::MapMode::Direct, 0x80 | ext_shift, 0x8f | ext_shift, 0x8000, 0xffff, memory::memory_unmapped);
      bus.map(Bus::MapMode::Direct, 0xc0 | ext_shift, 0xcf | ext_shift, 0x0000, 0xffff, memory::memory_unmapped);
    }
  }

  //PSRAM mapping
  unsigned psram_shift = (regs.r[0x05] << 4) | (regs.r[0x06] << 5);

  if(!regs.r[0x02]) {
    //LoROM mapping
    psram_shift <<= 1;
    unsigned psram_base = (psram_shift & 0x40) ? 0x0000 : 0x8000;
    
    if(regs.r[0x03]) {
      //Lower Banks (0x00-0x7D)
      bus.map(Bus::MapMode::Linear, 0x00 | psram_shift, 0x0f | psram_shift, psram_base, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Linear, 0x70, 0x7d, 0x0000, 0x7fff, memory::bsxpram);
    }

    if(regs.r[0x04]) {
      //Higher Banks (0x80-0xFF)
      bus.map(Bus::MapMode::Linear, 0x80 | psram_shift, 0x8f | psram_shift, psram_base, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Linear, 0xf0, 0xff, 0x0000, 0x7fff, memory::bsxpram);
    }
  } else {
    //HiROM mapping
    if(regs.r[0x03]) {
      //Lower Banks (0x00-0x7D)
      bus.map(Bus::MapMode::Shadow, 0x00 | psram_shift, 0x07 | psram_shift, 0x8000, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Linear, 0x40 | psram_shift, 0x47 | psram_shift, 0x0000, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Shadow, 0x20, 0x3f, 0x6000, 0x7fff, memory::bsxpram);
    }

    if(regs.r[0x04]) {
      //Higher Banks (0x80-0xFF)
      bus.map(Bus::MapMode::Shadow, 0x80 | psram_shift, 0x87 | psram_shift, 0x8000, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Linear, 0xc0 | psram_shift, 0xc7 | psram_shift, 0x0000, 0xffff, memory::bsxpram);
      bus.map(Bus::MapMode::Shadow, 0xa0, 0xbf, 0x6000, 0x7fff, memory::bsxpram);
    }
  }

  //BS-X ROM mapping
  if(regs.r[0x07]) {
    bus.map(Bus::MapMode::Linear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
  }

  if(regs.r[0x08]) {
    bus.map(Bus::MapMode::Linear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
  }
}

uint8 BSXCart::read(unsigned addr) {
  uint8 n = (addr >> 16) & 15;
  bool data = false;
  
  switch(n) {
  case 0x00: data = regs.irq; break;
  case 0x01: data = regs.irq_en; break;
  default:   data = regs.r[n]; break;
  case 0x0e: // 14 & 15 are write only
  case 0x0f: data = false; break;
  }
  
  return (data << 7) | (cpu.regs.mdr &= 0x7f);
}

void BSXCart::write(unsigned addr, uint8 data) {
  uint8 n = (addr >> 16) & 15;
  data >>= 7;
  
  switch(n) {
  case 0x00:
    regs.irq = false; // acknowledge "flash ready" IRQ (currently unused)
    break; 
  case 0x01:
    regs.irq_en = data; 
    break;
  default:
    // certain games (Treasure Conflix) write these registers way too frequently,
    // causing extremely excessive calls to bus.map() which makes emulation unplayably slow,
    // unless we avoid unnecessarily updating the memory map
    regs.rtemp[n] = data;
    if(data != regs.r[n])
      regs.dirty = true; 
    break;
  case 0x0e:
    if(regs.dirty) {
      memcpy(regs.r, regs.rtemp, sizeof(regs.r));
      update_memory_map();
      regs.dirty = false;
    }
    break;
  }
}

BSXCart::BSXCart() {
}

BSXCart::~BSXCart() {
}

#endif

