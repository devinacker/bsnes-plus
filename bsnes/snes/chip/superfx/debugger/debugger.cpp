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

  // TODO: all of these
  
  #undef item
  return false;
}

#endif
