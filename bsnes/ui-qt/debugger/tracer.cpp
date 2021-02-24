#include "tracer.moc"
Tracer *tracer;

void Tracer::stepCpu() {
  if(traceCpu) {
    unsigned addr = SNES::cpu.regs.pc;
    if(!traceMask || !(traceMaskCPU[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::cpu.disassemble_opcode(text, addr, config().debugger.showHClocks);
      tracefile.print(string() << text << "\n");
    }
    traceMaskCPU[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSmp() {
  if(traceSmp) {
    unsigned addr = SNES::smp.regs.pc;
    if(!traceMask || !(traceMaskSMP[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::smp.disassemble_opcode(text, addr);
      tracefile.print(string() << text << "\n");
    }
    traceMaskSMP[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSa1() {
  if(traceSa1) {
    unsigned addr = SNES::sa1.regs.pc;
    if(!traceMask || !(traceMaskSA1[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::sa1.disassemble_opcode(text, addr, config().debugger.showHClocks);
      tracefile.print(string() << text << "\n");
    }
    traceMaskSA1[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSfx() {
  if(traceSfx) {
    unsigned addr = SNES::superfx.opcode_pc;
    if(!traceMask || !(traceMaskSFX[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::superfx.disassemble_opcode(text, addr);
      tracefile.print(string() << text << "\n");
    }
    traceMaskSFX[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSgb() {
  if(traceSgb) {
    unsigned addr = SNES::supergameboy.opcode_pc;
    if(!traceMask || !(traceMaskSGB[addr >> 3] & (0x80 >> (addr & 7)))) {
      char text[256];
      SNES::supergameboy.disassemble_opcode(text, addr);
      tracefile.print(string() << text << "\n");
    }
    traceMaskSGB[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::resetTraceState() {
  tracefile.close();
  setTraceState(traceCpu || traceSmp || traceSa1 || traceSfx || traceSgb);

  // reset trace masks
  if (traceMask)
    setTraceMaskState(true);
}

void Tracer::setTraceState(bool state) {
  if(state && !tracefile.open() && SNES::cartridge.loaded()) {
    string name = filepath(nall::basename(cartridge.fileName), config().path.data);
    name << "-trace.log";
    tracefile.open(name, file::mode::write);
  } else if(!traceCpu && !traceSmp && !traceSa1 && !traceSfx && !traceSgb && tracefile.open()) {
    tracefile.close();
  }
}

void Tracer::setCpuTraceState(int state) {
  traceCpu = (state == Qt::Checked);
  setTraceState(traceCpu);
}

void Tracer::setSmpTraceState(int state) {
  traceSmp = (state == Qt::Checked);
  setTraceState(traceSmp);
}

void Tracer::setSa1TraceState(int state) {
  traceSa1 = (state == Qt::Checked);
  setTraceState(traceSa1);
}

void Tracer::setSfxTraceState(int state) {
  traceSfx = (state == Qt::Checked);
  setTraceState(traceSfx);
}

void Tracer::setSgbTraceState(int state) {
  traceSgb = (state == Qt::Checked);
  setTraceState(traceSgb);
}

void Tracer::setTraceMaskState(bool state) {
  traceMask = state;
  if(traceMask) {
    //flush all bitmasks once enabled
    memset(traceMaskCPU, 0x00, (1 << 24) >> 3);
    memset(traceMaskSMP, 0x00, (1 << 16) >> 3);
    memset(traceMaskSA1, 0x00, (1 << 24) >> 3);
    memset(traceMaskSFX, 0x00, (1 << 23) >> 3);
    memset(traceMaskSGB, 0x00, (1 << 24) >> 3);
  }
}

Tracer::Tracer() {
  traceCpu = false;
  traceSmp = false;
  traceSa1 = false;
  traceSfx = false;
  traceSgb = false;
  traceMask = false;

  traceMaskCPU = new uint8_t[(1 << 24) >> 3]();
  traceMaskSMP = new uint8_t[(1 << 16) >> 3]();
  traceMaskSA1 = new uint8_t[(1 << 24) >> 3]();
  traceMaskSFX = new uint8_t[(1 << 23) >> 3]();
  traceMaskSGB = new uint8_t[(1 << 24) >> 3]();

  SNES::cpu.step_event = { &Tracer::stepCpu, this };
  SNES::smp.step_event = { &Tracer::stepSmp, this };
  SNES::sa1.step_event = { &Tracer::stepSa1, this };
  SNES::superfx.step_event = { &Tracer::stepSfx, this };
  SNES::supergameboy.step_event = { &Tracer::stepSgb, this };
}

Tracer::~Tracer() {
  delete[] traceMaskCPU;
  delete[] traceMaskSMP;
  delete[] traceMaskSA1;
  delete[] traceMaskSFX;
  delete[] traceMaskSGB;
  if(tracefile.open()) tracefile.close();
}
