#ifdef SUPERFX_CPP

void SFXDebugger::op_step() {
  bool break_event = false;

  opcode_pc = regs.r[15] + (regs.pbr << 16);
  usage[opcode_pc] |= UsageExec;

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

  // adjust call count if this is a call or return
  // (or if we're stepping over and no call occurred)
  // (TODO: track interrupts as well?)
  
  // TODO: this is totally fucking broken for the superfx right now so it's just disabled
  /*(
  if (debugger.step_sfx) {
    if (debugger.step_over_new && debugger.call_count == 0) {
      debugger.call_count = -1;
      debugger.step_over_new = false;
    }
 
    // TODO: test these more for step out/over
    uint8 opcode = peekpipe();
    // link
    if (opcode >= 0x91 && opcode < 0x95) {
      debugger.call_count++;
    // jmp r11
    } else if (opcode == 0x9b && regs.sfr & 0x0100 == 0) {
      debugger.call_count--;
    }
  }
  */
  
  opcode_edge = false;
}

uint8 SFXDebugger::peekpipe() {
  int pc = regs.r[15] + (regs.pbr << 16);
  usage[pc] |= UsageExec;
  
  int offset = cartridge.rom_offset(pc);
  if (offset >= 0) (*cart_usage)[offset] |= UsageExec;
  
  return SuperFX::peekpipe();
}

uint8 SFXDebugger::pipe() {
  int pc = regs.r[15] + (regs.pbr << 16);
  usage[pc] |= UsageExec;
  
  int offset = cartridge.rom_offset(pc);
  if (offset >= 0) (*cart_usage)[offset] |= UsageExec;
  
  return SuperFX::pipe();
}

uint8 SFXDebugger::op_read(uint16 addr) {
  uint32 fulladdr = addr + (regs.pbr << 16);
  uint8 data = SuperFX::op_read(addr);
  
  usage[fulladdr] |= UsageRead;
  
  int offset = cartridge.rom_offset(fulladdr);
  if (offset >= 0) (*cart_usage)[offset] |= UsageRead;
  
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SFXBus, Debugger::Breakpoint::Mode::Read, fulladdr, data);
  return data;
}

// TODO: a way to hook writes and trigger write breakpoints

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
  item("Register R0", string("0x", hex<4>(regs.r[0])));
  item("Register R1", string("0x", hex<4>(regs.r[1])));
  item("Register R2", string("0x", hex<4>(regs.r[2])));
  item("Register R3", string("0x", hex<4>(regs.r[3])));
  item("Register R4", string("0x", hex<4>(regs.r[4])));
  item("Register R5", string("0x", hex<4>(regs.r[5])));
  item("Register R6", string("0x", hex<4>(regs.r[6])));
  item("Register R7", string("0x", hex<4>(regs.r[7])));
  item("Register R8", string("0x", hex<4>(regs.r[8])));
  item("Register R9", string("0x", hex<4>(regs.r[9])));
  item("Register R10", string("0x", hex<4>(regs.r[10])));
  item("Register R11", string("0x", hex<4>(regs.r[11])));
  item("Register R12", string("0x", hex<4>(regs.r[12])));
  item("Register R13", string("0x", hex<4>(regs.r[13])));
  item("Register R14", string("0x", hex<4>(regs.r[14])));
  item("Register R15", string("0x", hex<4>(regs.r[15])));
  
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
  
  switch (regs.scmr & 0x24) {
  case 0x00: ht = "128 pixels"; break;
  case 0x04: ht = "160 pixels"; break;
  case 0x20: ht = "192 pixels"; break;
  case 0x24: ht = "OBJ mode"; break;
  }
  
  item("Color Mode", md)
  item("Screen Height", ht);
  item("Game Pak WRAM Access", regs.scmr & 0x8 ? "Super FX" : "SNES")
  item("Game Pak ROM Access", regs.scmr & 0x10 ? "Super FX" : "SNES")
  
  item("$303b", "")
  item("Version Register (VCR)", string("0x", hex<2>(regs.vcr)))
  
  item("$303c", "")
  item("Game Pak RAM Bank", regs.rambr ? "0x71" : "0x70")
  
  item("$303e-$303f", "")
  item("Cache Base Register (CBR)", string("0x", hex<4>(regs.cbr)))

  #undef item
  return false;
}

#endif
