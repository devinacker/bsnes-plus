#ifdef SUPERGAMEBOY_CPP

SGBDebugger::SGBDebugger() {
  usage = 0; // TODO
  cart_usage = 0;
}

SGBDebugger::~SGBDebugger() {
//  delete[] usage;
//  delete[] cart_usage;
}

void SGBDebugger::init() {
  SuperGameBoy::init();
  if (opened()) {
    sgb_read_gb  = sym("sgb_read_gb");
    sgb_write_gb = sym("sgb_write_gb");
  }
}

uint8_t SGBDebugger::read_gb(uint16_t addr) {
  if (sgb_read_gb) return sgb_read_gb(addr);
  return 0;
}

void SGBDebugger::write_gb(uint16_t addr, uint8_t data) {
  if (sgb_write_gb) sgb_write_gb(addr, data);
}
  
#endif
