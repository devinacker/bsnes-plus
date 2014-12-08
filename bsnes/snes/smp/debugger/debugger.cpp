#ifdef SMP_CPP

void SMPDebugger::op_step() {
  bool break_event = false;

  usage[regs.pc] |= UsageExec;
  opcode_pc = regs.pc;

  opcode_edge = true;
  if(debugger.step_smp &&
      (debugger.step_type == Debugger::StepType::StepInto ||
       (debugger.step_type >= Debugger::StepType::StepOver && debugger.call_count < 0))) {
      
    debugger.break_event = Debugger::BreakEvent::SMPStep;
    debugger.step_type = Debugger::StepType::None;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::Source::APURAM, Debugger::Breakpoint::Mode::Exec, regs.pc, 0x00);
  }
  if(step_event) step_event();

  // adjust call count if this is a call or return
  // (or if we're stepping over and no call occurred)
  // (TODO: track interrupts as well?)
  if (debugger.step_smp) {
    if (debugger.step_over_new && debugger.call_count == 0) {
      debugger.call_count = -1;
      debugger.step_over_new = false;
    }
  
    uint8 opcode = SMP::op_read(opcode_pc);
    if (opcode == 0x3f || opcode == 0x4f || (opcode & 0xf) == 0x01) {
      debugger.call_count++;
    } else if (opcode == 0x6f) {
      debugger.call_count--;
    }
  }
  
  opcode_edge = false;

  SMP::op_step();
  synchronize_cpu();
}

alwaysinline uint8_t SMPDebugger::op_readpc() {
  usage[regs.pc] |= UsageExec;
  // execute code without setting read flag
  return SMP::op_read(regs.pc++);
}

uint8 SMPDebugger::op_read(uint16 addr) {
  uint8 data = SMP::op_read(addr);
  usage[addr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::APURAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void SMPDebugger::op_write(uint16 addr, uint8 data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::APURAM, Debugger::Breakpoint::Mode::Write, addr, data);
  SMP::op_write(addr, data);
  usage[addr] |= UsageWrite;
  usage[addr] &= ~UsageExec;
}

SMPDebugger::SMPDebugger() {
  usage = new uint8[1 << 16]();
  opcode_pc = 0xffc0;
  opcode_edge = false;
}

SMPDebugger::~SMPDebugger() {
  delete[] usage;
}

bool SMPDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  //$00f0
  item("$00f0", "");
  item("Clock Speed", (unsigned)status.clock_speed);
  item("Timers Enable", status.timers_enabled);
  item("RAM Disable", status.ram_disabled);
  item("RAM Writable", status.ram_writable);
  item("Timers Disable", status.timers_disabled);

  //$00f1
  item("$00f1", "");
  item("IPLROM Enable", status.iplrom_enabled);

  //$00f2
  item("$00f2", "");
  item("DSP Address", string("0x", hex<2>(status.dsp_addr)));

  #undef item
  return false;
}

#endif
