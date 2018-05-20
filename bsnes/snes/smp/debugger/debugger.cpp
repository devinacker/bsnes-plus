#ifdef SMP_CPP

void SMPDebugger::op_step() {
  bool break_event = false;

  usage[regs.pc] |= UsageOpcode;
  opcode_pc = regs.pc;

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
  
    uint8 opcode = memory::apuram[opcode_pc];
    if (opcode == 0x3f || opcode == 0x4f || (opcode & 0xf) == 0x01) {
      debugger.call_count++;
    } else if (opcode == 0x6f) {
      debugger.call_count--;
    }
  }
  
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
}

SMPDebugger::~SMPDebugger() {
  delete[] usage;
}

static string clockdivide(double base, unsigned divide) {
  if (!divide) divide = 256;
  return string(fp(base / divide), " Hz");
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
  item("Internal Speed", (unsigned)status.internal_speed);
  item("External Speed", (unsigned)status.external_speed);
  item("Timers Enable", status.timers_enabled);
  item("RAM Disable", status.ram_disabled);
  item("RAM Writable", status.ram_writable);
  item("Timers Disable", status.timers_disabled);

  //$00f1
  item("$00f1", "");
  item("IPLROM Enable", status.iplrom_enabled);

  //$00f2
  item("$00f2-$00f3", "");
  item("DSP Address", string("0x", hex<2>(status.dsp_addr)));
  item("DSP Data", string("0x", hex<2>(dsp.read(status.dsp_addr & 0x7f))))

  //$00f4
  item("$00f4", "");
  item("Port 0 CPU->SMP", string("0x", hex<2>(port.cpu_to_smp[0])));
  item("Port 0 SMP->CPU", string("0x", hex<2>(port.smp_to_cpu[0])));

  //$00f5
  item("$00f5", "");
  item("Port 1 CPU->SMP", string("0x", hex<2>(port.cpu_to_smp[1])));
  item("Port 1 SMP->CPU", string("0x", hex<2>(port.smp_to_cpu[1])));

  //$00f6
  item("$00f6", "");
  item("Port 2 CPU->SMP", string("0x", hex<2>(port.cpu_to_smp[2])));
  item("Port 2 SMP->CPU", string("0x", hex<2>(port.smp_to_cpu[2])));

  //$00f7
  item("$00f7", "");
  item("Port 3 CPU->SMP", string("0x", hex<2>(port.cpu_to_smp[3])));
  item("Port 3 SMP->CPU", string("0x", hex<2>(port.smp_to_cpu[3])));
  
  //$00fa
  item("$00fa", "");
  item("Timer 0 Divider", (unsigned)t0.target);
  item("Timer 0 Frequency", clockdivide(8000.0, (unsigned)t0.target));
  
  //$00fb
  item("$00fb", "");
  item("Timer 1 Divider", (unsigned)t1.target);
  item("Timer 1 Frequency", clockdivide(8000.0, (unsigned)t1.target));
  
  //$00fc
  item("$00fc", "");
  item("Timer 2 Divider", (unsigned)t2.target);
  item("Timer 2 Frequency", clockdivide(64000.0, (unsigned)t2.target));
  
  //$00fd-$00ff
  item("$00fd-$00ff", "");
  item("Timer 0 Count", (unsigned)t0.stage3_ticks);
  item("Timer 1 Count", (unsigned)t1.stage3_ticks);
  item("Timer 2 Count", (unsigned)t2.stage3_ticks);

  #undef item
  return false;
}

unsigned SMPDebugger::getRegister(unsigned id) {
  switch ((Register)id) {
  case RegisterPC: return regs.pc;
  case RegisterA:  return regs.a;
  case RegisterX:  return regs.x;
  case RegisterY:  return regs.y;
  case RegisterS:  return regs.sp;
  case RegisterYA: return regs.ya;
  case RegisterP:  return regs.p;
  }
  
  return 0;
}

void SMPDebugger::setRegister(unsigned id, unsigned value) {
  switch (id) {
  case RegisterPC: regs.pc = value; return;
  case RegisterA:  regs.a  = value; return;
  case RegisterX:  regs.x  = value; return;
  case RegisterY:  regs.y  = value; return;
  case RegisterS:  regs.sp = value; return;
  case RegisterYA: regs.ya = value; return;
  case RegisterP:  regs.p  = value; return;
  }
}

bool SMPDebugger::getFlag(unsigned id) {
  switch (id) {
  case FlagN: return regs.p.n;
  case FlagV: return regs.p.v;
  case FlagP: return regs.p.p;
  case FlagB: return regs.p.b;
  case FlagH: return regs.p.h;
  case FlagI: return regs.p.i;
  case FlagZ: return regs.p.z;
  case FlagC: return regs.p.c;
  }
  
  return false;
}

void SMPDebugger::setFlag(unsigned id, bool value) {
  switch (id) {
  case FlagN: regs.p.n = value; return;
  case FlagV: regs.p.v = value; return;
  case FlagP: regs.p.p = value; return;
  case FlagB: regs.p.b = value; return;
  case FlagH: regs.p.h = value; return;
  case FlagI: regs.p.i = value; return;
  case FlagZ: regs.p.z = value; return;
  case FlagC: regs.p.c = value; return;
  }
}

#endif
