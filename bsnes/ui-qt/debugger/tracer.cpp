#include "tracer.moc"
Tracer *tracer;

#include "w32_socket.cpp"

// TODO: demo only: make these checkboxes in the UI or config options

// tracer output info format
// false: binary format (small/faster),
// true:  text format (easier to parse / but slower and HUGE))
const bool traceOutputFormatIsText = false;

// where trace output will be sent
// true: listen on a socket port and stream data to a client
// false: output via a logfile on disk
const bool traceOutputMediumIsSocket = true;

#define DEFAULT_TRACE_SERVER_LISTEN_PORT "27015"

void Tracer::outputTrace(const char* buf, int len) {
    outputTraceToFile(buf, len);
    outputTraceToSocket(buf, len);
}

void Tracer::outputTraceToFile(const char *buf, int len) {
    if (!tracefile.open())
        return;

    // perf: without trask masking, this is SUPER SLOW, grinds the emulation to 2FPS when enabled.
    // TODO: there's probably some easy way to improve performance here, like buffering and async IO calls.
    // this chews on gigs of data quickly, so whatever you do, be mindful of performance.
    //
    tracefile.write(reinterpret_cast<const uint8_t *>(buf), len);
    if (traceOutputFormatIsText)
        tracefile.print("\n");
}

void Tracer::outputTraceToSocket(const char *buf, int len) {
    if (!traceServer.IsInitialized())
        return;

    traceServer.Push((const uint8_t*)buf, len);
}

void Tracer::outputCpuTrace() {
    char buf[256]; int len; // TODO: bounds check buf/len, make sure we don't overflow
    if (traceOutputFormatIsText) {
        SNES::cpu.disassemble_opcode(buf, SNES::cpu.regs.pc, config().debugger.showHClocks); // text
        len = strlen(buf) + 1; // null terminator
    } else {
        SNES::cpu.disassemble_opcode_bin(buf, SNES::cpu.regs.pc, len); // binary
    }
    outputTrace(buf, len);
}

void Tracer::outputSmpTrace() {
    char buf[256]; int len;
    if (traceOutputFormatIsText) {
        SNES::smp.disassemble_opcode(buf, SNES::cpu.regs.pc);
        len = strlen(buf) + 1; // byte size = string + null term
    } else {
        // TODO: implement // SNES::smp.disassemble_opcode_bin(buf, SNES::cpu.regs.pc, len); // binary
        return; // TODO: not supported just yet
    }
    outputTrace(buf, len);
}

void Tracer::outputSa1Trace() {
    char buf[256]; int len;
    if (traceOutputFormatIsText) {
        SNES::sa1.disassemble_opcode(buf, SNES::cpu.regs.pc, config().debugger.showHClocks);
        len = strlen(buf) + 1; // byte size = string + null term
    } else {
        // TODO: implement // SNES::sa1.disassemble_opcode_bin(buf, SNES::cpu.regs.pc, config().debugger.showHClocks, len); // binary
        return; // TODO: not supported just yet
    }
    outputTrace(buf, len);
}

void Tracer::outputSfxTrace() {
    char buf[256]; int len;
    if (traceOutputFormatIsText) {
        SNES::superfx.disassemble_opcode(buf, SNES::cpu.regs.pc);
        len = strlen(buf) + 1; // byte size = string + null term
    } else {
        // TODO: implement // SNES::superfx.disassemble_opcode_bin(buf, SNES::cpu.regs.pc, len); // binary
        return; // TODO: not supported just yet
    }
    outputTrace(buf, len);
}

void Tracer::outputSgbTrace() {
    char buf[256]; int len;
    if (traceOutputFormatIsText) {
        SNES::supergameboy.disassemble_opcode(buf, SNES::cpu.regs.pc);
        len = strlen(buf) + 1; // byte size = string + null term
    } else {
        // TODO: implement // SNES::supergameboy.disassemble_opcode_bin(buf, SNES::cpu.regs.pc, len); // binary
        return; // TODO: not supported just yet
    }
    outputTrace(buf, len);
}

void Tracer::stepCpu() {
  if(traceCpu) {
    unsigned addr = SNES::cpu.regs.pc;
    if(!traceMask || !(traceMaskCPU[addr >> 3] & (0x80 >> (addr & 7)))) {
        outputCpuTrace();
    }
    traceMaskCPU[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSmp() {
  if(traceSmp) {
    unsigned addr = SNES::smp.regs.pc;
    if(!traceMask || !(traceMaskSMP[addr >> 3] & (0x80 >> (addr & 7)))) {
        outputSmpTrace();
    }
    traceMaskSMP[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSa1() {
  if(traceSa1) {
    unsigned addr = SNES::sa1.regs.pc;
    if(!traceMask || !(traceMaskSA1[addr >> 3] & (0x80 >> (addr & 7)))) {
        outputSa1Trace();
    }
    traceMaskSA1[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSfx() {
  if(traceSfx) {
    unsigned addr = SNES::superfx.opcode_pc;
    if(!traceMask || !(traceMaskSFX[addr >> 3] & (0x80 >> (addr & 7)))) {
        outputSfxTrace();
    }
    traceMaskSFX[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::stepSgb() {
  if(traceSgb) {
    unsigned addr = SNES::supergameboy.opcode_pc;
    if(!traceMask || !(traceMaskSGB[addr >> 3] & (0x80 >> (addr & 7)))) {
        outputSgbTrace();
    }
    traceMaskSGB[addr >> 3] |= 0x80 >> (addr & 7);
  }
}

void Tracer::resetTraceState() {
  tracefile.close();
  traceServer.Shutdown();

  setTraceState(traceCpu || traceSmp || traceSa1 || traceSfx || traceSgb);

  // reset trace masks
  if (traceMask)
    setTraceMaskState(true);
}

void Tracer::ensureTraceOutputReady() {
    if (!traceOutputMediumIsSocket && !tracefile.open()) {
        string name = filepath(nall::basename(cartridge.fileName), config().path.data);
        name << "-trace.log";
        tracefile.open(name, file::mode::write);
    }

    if (traceOutputMediumIsSocket && !traceServer.IsInitialized()) {
        // demo: this blocks the entire UI while waiting for a client to connect (not great, better ways to handle)
        traceServer.Init(DEFAULT_TRACE_SERVER_LISTEN_PORT);
    }
}

void Tracer::ensureTraceOutputShutdown() {
    if (tracefile.open()) {
        tracefile.close();
    }

    if (traceServer.IsInitialized()) {
        traceServer.Shutdown();
    }
}

void Tracer::setTraceState(bool state) {
  if(state && SNES::cartridge.loaded()) {
      ensureTraceOutputReady();
  } else if(!traceCpu && !traceSmp && !traceSa1 && !traceSfx && !traceSgb) {
      ensureTraceOutputShutdown();
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

void Tracer::flushTraceOutput() {
    traceServer.FlushWorkingBuffer();
}
