#ifdef SUPERFX_CPP

void SFXDebugger::op_step() {
  bool break_event = false;

  // subtract 1 since r15 will have already advanced one byte
  // (putting the current opcode in the pipeline)
  opcode_pc = (regs.pbr << 16) + regs.r[15] - 1;
  usage[opcode_pc] |= UsageOpcode;

  opcode_edge = true;
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
  
  opcode_edge = false;
}

uint8 SFXDebugger::op_read(uint16 addr) {
  uint32 fulladdr = addr + (regs.pbr << 16);
  usage[fulladdr] |= UsageExec;
  
  int offset = cartridge.rom_offset(fulladdr);
  if (offset >= 0) (*cart_usage)[offset] |= UsageExec;
  
  return SuperFX::op_read(addr);
}

uint8 SFXDebugger::rombuffer_read() {
  uint32 fulladdr = (regs.rombr << 16) + regs.r[14];
  usage[fulladdr] |= UsageRead;
  
  uint8 data = sfxdebugbus.read(fulladdr);
  
  int offset = cartridge.rom_offset(fulladdr);
  if (offset >= 0) (*cart_usage)[offset] |= UsageRead;
  
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SFXBus, Debugger::Breakpoint::Mode::Read, fulladdr, data);
  return SuperFX::rombuffer_read();
}

uint8 SFXDebugger::rambuffer_read(uint16 addr) {
  uint32 fulladdr = 0x700000 + (regs.rambr << 16) + addr;
  usage[fulladdr] |= UsageRead;
  
  uint8 data = sfxdebugbus.read(fulladdr);
  
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
  usage = new uint8[1 << 24]();
  cart_usage = &SNES::cpu.cart_usage;
  opcode_pc = 0;
  opcode_edge = false;
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
  
  item("$3037", "")
  item("Multiplier Speed", (regs.cfgr & 0x20) ? "High Speed" : "Normal")
  item("IRQ Enable", (regs.cfgr & 0x80) ? "Disabled" : "Enabled")
  
  item("$3038", "")
  item("Screen Base Register (SCBR)", string("0x", hex<2>(regs.scbr)))
  
  item("$3039", "")
  item("Clock Register (CLSR)", regs.clsr ? "21.4 MHz" : "10.7 MHz")
  
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

#endif
