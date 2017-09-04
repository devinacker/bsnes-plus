#ifdef SUPERFX_CPP

void SFXDebugger::reset() {
  SuperFX::reset();
  
  pc_valid = false;
  opcode_pc = 0;
}

void SFXDebugger::op_step() {
  if (pc_valid) {
    usage[opcode_pc] |= UsageOpcode;

    if(debugger.step_sfx &&
        (debugger.step_type == Debugger::StepType::StepInto ||
         (debugger.step_type >= Debugger::StepType::StepOver && debugger.call_count < 0))) {
      
      debugger.break_event = Debugger::BreakEvent::SFXStep;
      debugger.step_type = Debugger::StepType::None;
      scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
    } else {
      debugger.breakpoint_test(Debugger::Breakpoint::Source::SFXBus, Debugger::Breakpoint::Mode::Exec, opcode_pc, 0x00);
    }
    if(step_event) step_event();
  }
}

uint8 SFXDebugger::op_read(uint16 addr) {
  pc_valid = true;
  opcode_pc = addr + (regs.pbr << 16);
  usage[opcode_pc] |= UsageExec;
  
  int offset = cartridge.rom_offset(opcode_pc);
  if (offset >= 0) (*cart_usage)[offset] |= UsageExec;
  
  return SuperFX::op_read(addr);
}

uint8 SFXDebugger::rombuffer_read() {
  uint32 fulladdr = (regs.rombr << 16) + regs.r[14];
  usage[fulladdr] |= UsageRead;
  
  SNES::debugger.bus_access = true;
  uint8 data = superfxbus.read(fulladdr);
  SNES::debugger.bus_access = false;
  
  int offset = cartridge.rom_offset(fulladdr);
  if (offset >= 0) (*cart_usage)[offset] |= UsageRead;
  
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SFXBus, Debugger::Breakpoint::Mode::Read, fulladdr, data);
  return SuperFX::rombuffer_read();
}

uint8 SFXDebugger::rambuffer_read(uint16 addr) {
  uint32 fulladdr = 0x700000 + (regs.rambr << 16) + addr;
  usage[fulladdr] |= UsageRead;
  
  SNES::debugger.bus_access = true;
  uint8 data = superfxbus.read(fulladdr);
  SNES::debugger.bus_access = false;
  
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SFXBus, Debugger::Breakpoint::Mode::Read, fulladdr, data);
  return SuperFX::rambuffer_read(addr);
}

void SFXDebugger::rambuffer_write(uint16 addr, uint8 data) {
  uint32 fulladdr = 0x700000 + (regs.rambr << 16) + addr;
  usage[fulladdr] |= UsageWrite;
  
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SFXBus, Debugger::Breakpoint::Mode::Write, fulladdr, data);
  SuperFX::rambuffer_write(addr, data);
}

SFXDebugger::SFXDebugger() {
  usage = new uint8[1 << 23]();
  cart_usage = &SNES::cpu.cart_usage;
}

SFXDebugger::~SFXDebugger() {
  delete[] usage;
}

bool SFXDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  item("$3000-$301f", "")
  for (int i = 0; i < 16; i++) {
    item(string("Register R", i), string("0x", hex<4>(regs.r[i])))
  }
  
  item("$3030", "")
  item("Status Flag Register (SFR)", string("0x", hex<4>(regs.sfr)))
  
  item("$3033", "")
  item("Backup RAM Register (BRAMR)", string("0x", hex<2>(regs.bramr)))

  item("$3034", "")
  item("Program Bank Register", string("0x", hex<2>(regs.pbr)))
  
  item("$3036", "")
  item("Rom Bank Register", string("0x", hex<2>(regs.rombr)))

  item("$3037", "")
  item("Multiplier Speed", (regs.cfgr & 0x20) ? "High Speed" : "Normal")
  item("IRQ Enable", (regs.cfgr & 0x80) ? "Disabled" : "Enabled")
  
  item("$3038", "")
  item("Screen Base Register (SCBR)", string("0x", hex<2>(regs.scbr)))
  
  item("$3039", "")
  item("Clock Register (CLSR)", regs.clsr.divider == 1 ? "21.4 MHz" : "10.7 MHz")
  
  item("$303a", "")
  string md, ht;
  
  switch (regs.scmr & 0x3) {
  case 0: md = "4 colors"; break;
  case 1: md = "16 colors"; break;
  case 2: md = "Invalid"; break;
  case 3: md = "256 colors"; break;
  }
  
  if (regs.por & 0x10) {
    ht = "OBJ mode";
  } else switch (regs.scmr & 0x24) {
  case 0x00: ht = "128 pixels"; break;
  case 0x04: ht = "160 pixels"; break;
  case 0x20: ht = "192 pixels"; break;
  case 0x24: ht = "OBJ mode"; break;
  }
  
  item("Color Mode", md)
  item("Screen Height", ht);
  item("Game Pak WRAM Access", (regs.scmr & 0x8) ? "Super FX" : "SNES")
  item("Game Pak ROM Access", (regs.scmr & 0x10) ? "Super FX" : "SNES")
  
  item("$303b", "")
  item("Version Register (VCR)", string("0x", hex<2>(regs.vcr)))
  
  item("$303c", "")
  item("Game Pak RAM Bank", regs.rambr ? "0x71" : "0x70")
  
  item("$303e-$303f", "")
  item("Cache Base Register (CBR)", string("0x", hex<4>(regs.cbr)))
  
  item("", "")
  item("Color Data", string(regs.colr, " (0x", hex<2>(regs.colr), ")"))
  item("Plot Transparent", (regs.por & 0x1) ? "Disabled" : "Enabled")
  item("Plot Dither", (regs.por & 0x2) ? "Enabled" : "Disabled")
  item("COLOR/GETC High Nibble", (regs.por & 0x4) ? "Enabled" : "Disabled")
  item("COLOR/GETC Freeze High", (regs.por & 0x8) ? "Enabled" : "Disabled")

  #undef item
  return false;
}

unsigned SFXDebugger::getRegister(unsigned id) {
  if (id < 16) {
    return regs.r[id];
  } else switch ((Register)id) {
  case RegisterSFR:  return regs.sfr;
  }
  
  return 0;
}

void SFXDebugger::setRegister(unsigned id, unsigned value) {
  if (id < 16) {
    regs.r[id] = value; return;
  } else switch ((Register)id) {
  case RegisterSFR:  regs.sfr = value; return;
  }
}

bool SFXDebugger::getFlag(unsigned id) {
  switch (id) {
  case FlagI:  return regs.sfr.irq;
  case FlagB:  return regs.sfr.b;
  case FlagIH: return regs.sfr.ih;
  case FlagIL: return regs.sfr.il;
  case FlagA2: return regs.sfr.alt2;
  case FlagA1: return regs.sfr.alt1;
  case FlagR:  return regs.sfr.r;
  case FlagG:  return regs.sfr.g;
  case FlagV:  return regs.sfr.ov;
  case FlagN:  return regs.sfr.s;
  case FlagC:  return regs.sfr.cy;
  case FlagZ:  return regs.sfr.z;
  }
  
  return false;
}

void SFXDebugger::setFlag(unsigned id, bool value) {
  switch (id) {
  case FlagI:  regs.sfr.irq  = value; return;
  case FlagB:  regs.sfr.b    = value; return;
  case FlagIH: regs.sfr.ih   = value; return;
  case FlagIL: regs.sfr.il   = value; return;
  case FlagA2: regs.sfr.alt2 = value; return;
  case FlagA1: regs.sfr.alt1 = value; return;
  case FlagR:  regs.sfr.r    = value; return;
  case FlagG:  regs.sfr.g    = value; return;
  case FlagV:  regs.sfr.ov   = value; return;
  case FlagN:  regs.sfr.s    = value; return;
  case FlagC:  regs.sfr.cy   = value; return;
  case FlagZ:  regs.sfr.z    = value; return;
  }
}

#endif
