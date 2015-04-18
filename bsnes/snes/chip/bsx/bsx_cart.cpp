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
  for(unsigned i = 0; i < 16; i++) regs.r[i] = 0x00;
  regs.r[0x07] = 0x80;
  regs.r[0x08] = 0x80;

  update_memory_map();
}

void BSXCart::update_memory_map() {
  Memory &cart = (regs.r[0x01] & 0x80) == 0x00 ? cartridge.bsxpack_access() : (Memory&)memory::bsxpram;

  if((regs.r[0x02] & 0x80) == 0x00) {
    //LoROM mapping
    bus.map(Bus::MapMode::Linear, 0x00, 0x7d, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0x80, 0xff, 0x8000, 0xffff, cart);
  } else {
    //HiROM mapping
    bus.map(Bus::MapMode::Shadow, 0x00, 0x3f, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0x40, 0x7d, 0x0000, 0xffff, cart);
    bus.map(Bus::MapMode::Shadow, 0x80, 0xbf, 0x8000, 0xffff, cart);
    bus.map(Bus::MapMode::Linear, 0xc0, 0xff, 0x0000, 0xffff, cart);
  }

  if(regs.r[0x03] & 0x80) {
    bus.map(Bus::MapMode::Linear, 0x60, 0x6f, 0x0000, 0xffff, memory::bsxpram);
  //bus.map(Bus::MapMode::Linear, 0x70, 0x77, 0x0000, 0xffff, memory::bsxpram);
  }

  if((regs.r[0x05] & 0x80) == 0x00) {
    bus.map(Bus::MapMode::Linear, 0x40, 0x4f, 0x0000, 0xffff, memory::bsxpram);
  }

  if((regs.r[0x06] & 0x80) == 0x00) {
    bus.map(Bus::MapMode::Linear, 0x50, 0x5f, 0x0000, 0xffff, memory::bsxpram);
  }

  if(regs.r[0x07] & 0x80) {
    bus.map(Bus::MapMode::Linear, 0x00, 0x1f, 0x8000, 0xffff, memory::cartrom);
  }

  if(regs.r[0x08] & 0x80) {
    bus.map(Bus::MapMode::Linear, 0x80, 0x9f, 0x8000, 0xffff, memory::cartrom);
  }

  bus.map(Bus::MapMode::Shadow, 0x20, 0x3f, 0x6000, 0x7fff, memory::bsxpram);
  bus.map(Bus::MapMode::Linear, 0x70, 0x77, 0x0000, 0xffff, memory::bsxpram);
}

uint8 BSXCart::read(unsigned addr) {
  uint8 n = (addr >> 16) & 15;
  return regs.r[n];
}

void BSXCart::write(unsigned addr, uint8 data) {
  uint8 n = (addr >> 16) & 15;
  regs.r[n] = data;
  if(n == 0x0e && data & 0x80) update_memory_map();
}

BSXCart::BSXCart() {
}

BSXCart::~BSXCart() {
}

#endif

