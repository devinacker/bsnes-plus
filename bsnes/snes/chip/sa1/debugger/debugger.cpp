#ifdef SA1_CPP

uint8 SA1Debugger::disassembler_read(uint32 addr)
{
  debugger.bus_access = true;
  uint8 data = sa1bus.read(addr);
  debugger.bus_access = false;
  return data;
}

void SA1Debugger::op_step() {
  bool break_event = false;

  usage[regs.pc] &= ~(UsageFlagM | UsageFlagX);
  usage[regs.pc] |= UsageOpcode | (regs.p.m << 1) | (regs.p.x << 0);
  opcode_pc = regs.pc;

  if(debugger.step_sa1 &&
      (debugger.step_type == Debugger::StepType::StepInto ||
       (debugger.step_type >= Debugger::StepType::StepOver && debugger.call_count < 0))) {
      
    debugger.break_event = Debugger::BreakEvent::SA1Step;
    debugger.step_type = Debugger::StepType::None;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  } else {
        
    if (debugger.break_on_wdm) {
      uint8 opcode = disassembler_read(opcode_pc);
      if (opcode == 0x42) {
        debugger.breakpoint_hit = Debugger::SoftBreakSA1;
        debugger.break_event = Debugger::BreakEvent::BreakpointHit;
        scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
      }
    }
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
  
    uint8 opcode = disassembler_read(opcode_pc);
    if (opcode == 0x20 || opcode == 0x22 || opcode == 0xfc) {
      debugger.call_count++;
    } else if (opcode == 0x60 || opcode == 0x6b) {
      debugger.call_count--;
    }
  }
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
  // ignore dummy reads that can be caused by interrupts
  if (!interrupt_pending()) {
    usage[addr] |= UsageRead;
  
    int offset = cartridge.rom_offset(addr);
    if (offset >= 0) (*cart_usage)[offset] |= UsageRead;
  
    debugger.breakpoint_test(Debugger::Breakpoint::Source::SA1Bus, Debugger::Breakpoint::Mode::Read, addr, data);
  }
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

  // internal
  item("SA-1 MDR", string("0x", hex<2>(regs.mdr)));
  
  // $2200
  item("$2200", "");
  item("SA-1 IRQ", mmio.sa1_irq);
  item("SA-1 Ready", mmio.sa1_rdyb);
  item("SA-1 Reset", mmio.sa1_resb);
  item("SA-1 NMI", mmio.sa1_nmi);
  item("S-CPU to SA-1 Message", string("0x", hex<1>(mmio.smeg)));
  
  // $2201
  item("$2201", "");
  item("S-CPU IRQ", mmio.cpu_irqen);
  item("DMA IRQ", mmio.chdma_irqen);
  
  // $2203-2208
  item("$2203-$2204", "");
  item("SA-1 Reset Vector", string("0x", hex<4>(mmio.crv)));
  item("$2205-$2206", "");
  item("SA-1 NMI Vector", string("0x", hex<4>(mmio.cnv)));
  item("$2207-$2208", "");
  item("SA-1 IRQ Vector", string("0x", hex<4>(mmio.civ)));
  
  // $220c-220f
  item("$220c-$220d", "");
  item("S-CPU NMI Vector", string("0x", hex<4>(mmio.snv)));
  item("$220e-$220f", "");
  item("S-CPU IRQ Vector", string("0x", hex<4>(mmio.siv)));
  
  // $2210
  item("$2210", "");
  item("Timer Type", mmio.hvselb ? "Linear" : "H/V");
  item("V-Count Enable", mmio.ven);
  item("H-Count Enable", mmio.hen);
  
  // $2220-2223
  item("$2220", "");
  item("Bank C Projection", mmio.cbmode);
  item("Bank C ($00-$1F)", string("0x", hex<1>(mmio.cb), " (0x", hex<6>(mmio.cb << 20), ")"));
  
  item("$2221", "");
  item("Bank D Projection", mmio.dbmode);
  item("Bank D ($20-$3F)", string("0x", hex<1>(mmio.db), " (0x", hex<6>(mmio.db << 20), ")"));
  
  item("$2222", "");
  item("Bank E Projection", mmio.ebmode);
  item("Bank E ($80-$9F)", string("0x", hex<1>(mmio.eb), " (0x", hex<6>(mmio.eb << 20), ")"));
  
  item("$2223", "");
  item("Bank F Projection", mmio.fbmode);
  item("Bank F ($a0-$bF)", string("0x", hex<1>(mmio.fb), " (0x", hex<6>(mmio.fb << 20), ")"));
  
  // TODO: rest of these
  
  #undef item
  return false;
}

unsigned SA1Debugger::getRegister(unsigned id) {
  switch (id) {
  case RegisterPC: return regs.pc;
  case RegisterA:  return regs.a;
  case RegisterX:  return regs.x;
  case RegisterY:  return regs.y;
  case RegisterS:  return regs.s;
  case RegisterD:  return regs.d;
  case RegisterDB: return regs.db;
  case RegisterP:  return regs.p;
  }
  
  return 0;
}

void SA1Debugger::setRegister(unsigned id, unsigned value) {
  switch (id) {
  case RegisterPC: regs.pc = value; return;
  case RegisterA:  regs.a  = value; return;
  case RegisterX:  regs.x  = regs.p.x ? (value & 0xff) : value; return;
  case RegisterY:  regs.y  = regs.p.x ? (value & 0xff) : value; return;
  case RegisterS:  regs.s  = value; return;
  case RegisterD:  regs.d  = value; return;
  case RegisterDB: regs.db = value; return;
  case RegisterP:  regs.p  = value; return;
  }
}

bool SA1Debugger::getFlag(unsigned id) {
  switch (id) {
  case FlagE: return regs.e;
  case FlagN: return regs.p.n;
  case FlagV: return regs.p.v;
  case FlagM: return regs.p.m;
  case FlagX: return regs.p.x;
  case FlagD: return regs.p.d;
  case FlagI: return regs.p.i;
  case FlagZ: return regs.p.z;
  case FlagC: return regs.p.c;
  }
  
  return false;
}

void SA1Debugger::setFlag(unsigned id, bool value) {
  switch (id) {
  case FlagE: regs.e   = value; return;
  case FlagN: regs.p.n = value; return;
  case FlagV: regs.p.v = value; return;
  case FlagD: regs.p.d = value; return;
  case FlagI: regs.p.i = value; return;
  case FlagZ: regs.p.z = value; return;
  case FlagC: regs.p.c = value; return;
  case FlagM: 
    regs.p.m = value; 
    update_table();
    return;
  case FlagX: 
    regs.p.x = value;
    if (value)
      regs.x.h = regs.y.h = 0; 
    update_table();
    return;
  }
}

#endif
