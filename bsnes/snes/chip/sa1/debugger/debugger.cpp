#ifdef SA1_CPP

void SA1Debugger::op_step() {
  bool break_event = false;

  usage[regs.pc] &= ~(UsageFlagM | UsageFlagX);
  usage[regs.pc] |= UsageExec | (regs.p.m << 1) | (regs.p.x << 0);
  opcode_pc = regs.pc;

  opcode_edge = true;
  if(debugger.step_sa1 &&
      (debugger.step_type == Debugger::StepType::StepInto ||
       (debugger.step_type >= Debugger::StepType::StepOver && debugger.call_count < 0))) {
      
    debugger.break_event = Debugger::BreakEvent::SA1Step;
    debugger.step_type = Debugger::StepType::None;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::Source::SA1Bus, Debugger::Breakpoint::Mode::Exec, regs.pc, 0x00);
  }
  if(step_event) step_event();

  // adjust call count if this is a call or return
  // (or if we're stepping over and no call occurred)
  // (TODO: track interrupts as well?)
  if (debugger.step_sa1) {
    if (debugger.step_over_new && debugger.call_count == 0) {
      debugger.call_count = -1;
      debugger.step_over_new = false;
    }
  
    uint8 opcode = SA1::op_read(opcode_pc);
    if (opcode == 0x20 || opcode == 0x22 || opcode == 0xfc) {
      debugger.call_count++;
    } else if (opcode == 0x60 || opcode == 0x6b) {
      debugger.call_count--;
    }
  }
  
  opcode_edge = false;
}

alwaysinline uint8_t SA1Debugger::op_readpc() {
  usage[regs.pc] |= UsageExec;
  
  int offset = cartridge.rom_offset(regs.pc);
  if (offset >= 0) (*cart_usage)[offset] |= UsageExec;
  
  // execute code without setting read flag
  return SA1::op_read((regs.pc.b << 16) + regs.pc.w++);
}

uint8 SA1Debugger::op_read(uint32 addr) {
  uint8 data = SA1::op_read(addr);
  usage[addr] |= UsageRead;
  
  int offset = cartridge.rom_offset(addr);
  if (offset >= 0) (*cart_usage)[offset] |= UsageRead;
  
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SA1Bus, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

// TODO: SA-1 DMA/HDMA

void SA1Debugger::op_write(uint32 addr, uint8 data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SA1Bus, Debugger::Breakpoint::Mode::Write, addr, data);
  SA1::op_write(addr, data);
  usage[addr] |= UsageWrite;
  usage[addr] &= ~UsageExec;
}

// TODO: SA-1 MMIO, bitmap RAM access and stuff

SA1Debugger::SA1Debugger() {
  usage = new uint8[1 << 24]();
  cart_usage = &SNES::cpu.cart_usage;
  opcode_pc = 0x8000;
  opcode_edge = false;
}

SA1Debugger::~SA1Debugger() {
  delete[] usage;
}

bool SA1Debugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  // TODO: these

  #undef item
  return false;
}

#endif
