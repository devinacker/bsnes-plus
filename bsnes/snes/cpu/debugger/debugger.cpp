/*
 * Building with the "performance" profile will include this from snes/alt/cpu/cpu.cpp instead of
 * the usual snes/cpu/cpu.cpp. When this is the case, ALT_CPU_CPP is defined.
 * Be sure to test builds with multiple profiles and account for differences in the two implementations.
 */
#ifdef CPU_CPP

uint8 CPUDebugger::disassembler_read(uint32 addr)
{
  debugger.bus_access = true;
  uint8 data = bus.read(addr);
  debugger.bus_access = false;
  return data;
}

uint8 CPUDebugger::hvbjoy()
{
#if defined(ALT_CPU_CPP)
  return mmio_read(0x4212);
#else
  return mmio_r4212();
#endif
}

#ifdef ALT_CPU_CPP
void CPUDebugger::op_irq(uint16 vector) {
  CPU::op_irq(vector);
#else
void CPUDebugger::op_irq() {
  CPU::op_irq();
  const auto& vector = status.interrupt_vector;
#endif
  if (debugger.step_cpu) {
    debugger.call_count++;
    
    if ((debugger.step_type == Debugger::StepType::StepToNMI && (vector & 0xf) == 0xa)
        || (debugger.step_type == Debugger::StepType::StepToIRQ && (vector & 0xf) == 0xe)) {
      // break on next instruction after interrupt
      debugger.step_type = Debugger::StepType::StepInto;
    }
  }
}

void CPUDebugger::op_step() {
  usage[regs.pc] &= ~(UsageFlagM | UsageFlagX);
  usage[regs.pc] |= UsageOpcode | (regs.p.m << 1) | (regs.p.x << 0);
  opcode_pc = regs.pc;

  if(debugger.step_cpu &&
      (debugger.step_type == Debugger::StepType::StepInto ||
       (debugger.step_type >= Debugger::StepType::StepOver && debugger.call_count < 0))) {

    debugger.break_event = Debugger::BreakEvent::CPUStep;
    debugger.step_type = Debugger::StepType::None;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  } else {

    if (debugger.break_on_wdm || debugger.break_on_brk) {
      uint8 opcode = disassembler_read(opcode_pc);
      if ((opcode == 0x42 && debugger.break_on_wdm) || (opcode == 0x00 && debugger.break_on_brk)) {
        debugger.breakpoint_hit = Debugger::SoftBreakCPU;
        debugger.break_event = Debugger::BreakEvent::BreakpointHit;
        scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
      }
    }
    debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Exec, regs.pc, 0x00);
  }
  if(step_event) step_event();

  uint8 hvb_old;

  // adjust call count if this is a call or return
  // (or if we're stepping over and no call occurred)
  if (debugger.step_cpu) {
    if (debugger.step_over_new && debugger.call_count == 0) {
      debugger.call_count = -1;
      debugger.step_over_new = false;
    }
  
    uint8 opcode = disassembler_read(opcode_pc);
    if (opcode == 0x20 || opcode == 0x22 || opcode == 0xfc) {
      debugger.call_count++;
    } else if (opcode == 0x60 || opcode == 0x6b || opcode == 0x40) {
      debugger.call_count--;
    }

    hvb_old = hvbjoy();
  }

  CPU::op_step();
  synchronize_smp();
  
  if (debugger.step_cpu) {
    uint8 hvb_new = hvbjoy();
    if ((debugger.step_type == Debugger::StepType::StepToVBlank && !(hvb_old & 0x80) && (hvb_new & 0x80))
        || (debugger.step_type == Debugger::StepType::StepToHBlank && !(hvb_old & 0x40) && (hvb_new & 0x40))) {
      // break on next instruction after vblank/hblank
      debugger.step_type = Debugger::StepType::StepInto;
    }
  }
}

alwaysinline uint8_t CPUDebugger::op_readpc() {
  usage[regs.pc] |= UsageExec;
  
  int offset = cartridge.rom_offset(regs.pc);
  if (offset >= 0) cart_usage[offset] |= UsageExec;
  
  // execute code without setting read flag
  return CPU::op_read((regs.pc.b << 16) + regs.pc.w++);
}

uint8 CPUDebugger::op_read(uint32 addr) {
  uint8 data = CPU::op_read(addr);
  // ignore dummy reads that can be caused by interrupts
  if (!interrupt_pending()) {
    usage[addr] |= UsageRead;
  
    int offset = cartridge.rom_offset(addr);
    if (offset >= 0) cart_usage[offset] |= UsageRead;
  
    debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Read, addr, data);
  }
  return data;
}

uint8 CPUDebugger::dma_read(uint32 abus) {
  usage[abus] |= UsageRead;
  
  int offset = cartridge.rom_offset(abus);
  if (offset >= 0) cart_usage[offset] |= UsageRead;
  
  uint8 data = CPU::dma_read(abus);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Read, abus, data);
  return data;
}

void CPUDebugger::op_write(uint32 addr, uint8 data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Write, addr, data);
  CPU::op_write(addr, data);
  usage[addr] |= UsageWrite;
  usage[addr] &= ~UsageExec;
}

// $2180 MMIO-based WRAM access
#if defined(ALT_CPU_CPP)
uint8 CPUDebugger::mmio_read(unsigned addr) {
  if (addr & 0xffff == 0x2180) {
    uint32 fulladdr = 0x7e0000 | status.wram_addr;
    uint8 data = bus.read(fulladdr);
  
    usage[fulladdr] |= UsageRead;
    debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Read, fulladdr, data);
  }
  
  return CPU::mmio_read(addr);
}

void CPUDebugger::mmio_write(unsigned addr, uint8 data) {
  if (addr & 0xffff == 0x2180) {
    uint32 fulladdr = 0x7e0000 | status.wram_addr;
  
    usage[fulladdr] |= UsageWrite;
    debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Write, fulladdr, data);
  }
  
  CPU::mmio_write(addr, data);
}
#else
uint8 CPUDebugger::mmio_r2180() {
  uint32 fulladdr = 0x7e0000 | status.wram_addr;
  uint8 data = bus.read(fulladdr);
 
  usage[fulladdr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Read, fulladdr, data);
  return CPU::mmio_r2180();
}

void CPUDebugger::mmio_w2180(uint8 data) {
  uint32 fulladdr = 0x7e0000 | status.wram_addr;
 
  usage[fulladdr] |= UsageWrite;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CPUBus, Debugger::Breakpoint::Mode::Write, fulladdr, data);
  CPU::mmio_w2180(data);
}
#endif

CPUDebugger::CPUDebugger() {
  usage = new uint8[1 << 24]();
  cart_usage = new uint8[1 << 24]();
  opcode_pc = 0x8000;
}

CPUDebugger::~CPUDebugger() {
  delete[] usage;
  delete[] cart_usage;
}

bool CPUDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  //internal
  item("S-CPU open bus", string("0x", hex<2>(regs.mdr)));

  //$2181-2183
  item("$2181-$2183", "");
  item("WRAM Address", string("0x", hex<6>(status.wram_addr)));

  //$4016
  item("$4016", "");
  item("Joypad Strobe Latch", status.joypad_strobe_latch);

  //$4200
  item("$4200", "");
  item("NMI Enable", status.nmi_enabled);
  item("H-Count IRQ Enable", status.hirq_enabled);
  item("V-Count IRQ Enable", status.virq_enabled);
  item("Auto Joypad Poll", status.auto_joypad_poll);

  //$4201
  item("$4201", "");
  item("PIO", string("0x", hex<2>(status.pio)));

  //$4202
  item("$4202", "");
  item("Multiplicand", string(status.wrmpya, " (0x", hex<2>(status.wrmpya), ")"));

  //$4203
  item("$4203", "");
  item("Multiplier", string(status.wrmpyb, " (0x", hex<2>(status.wrmpyb), ")"));

  //$4204-$4205
  item("$4204-$4205", "");
  item("Dividend", string(status.wrdiva, " (0x", hex<4>(status.wrdiva), ")"));

  //$4206
  item("$4206", "");
  item("Divisor", string(status.wrdivb, " (0x", hex<2>(status.wrdivb), ")"));

  //$4207-$4208
  item("$4207-$4208", "");
  item("H-Count", string("0x", hex<4>(status.hirq_pos)));

  //$4209-$420a
  item("$4209-$420a", "");
  item("V-Count", string("0x", hex<4>(status.virq_pos)));

  //$420b
  unsigned dma_enable = 0;
  for(unsigned n = 0; n < 8; n++) dma_enable |= channel[n].dma_enabled << n;

  item("$420b", "");
  item("DMA Enable", string("0x", hex<2>(dma_enable)));

  //$420c
  unsigned hdma_enable = 0;
  for(unsigned n = 0; n < 8; n++) hdma_enable |= channel[n].hdma_enabled << n;

  item("$420c", "");
  item("HDMA Enable", string("0x", hex<2>(hdma_enable)));

  //$420d
  item("$420d", "");
  item("FastROM Enable", status.rom_speed == 6);

  //$4210
  item("$4210", "");
  item("NMI Flag", status.nmi_line);
#if defined(ALT_CPU_CPP)
  item("S-CPU Version", 2u);
#else
  item("S-CPU Version", (unsigned)cpu_version);
#endif

  //$4211
  item("$4211", "");
  item("IRQ Flag", status.irq_line);
  
  //$4212
  {
  uint8 r = hvbjoy();
  item("$4212", "");
  item("V-Blank Flag", (r & 0x80) != 0);
  item("H-Blank Flag", (r & 0x40) != 0);
  item("Auto Joypad Read", (r & 0x01) != 0);
  }
  
  //$4214-$4215
  item("$4214-$4215", "");
  item("Quotient", string((unsigned)status.rddiv, " (0x", hex<4>(status.rddiv), ")"));
  
  item("$4216-$4217", "");
  item("Product / Remainder", string((unsigned)status.rdmpy, " (0x", hex<4>(status.rdmpy), ")"));
  
  item("$4218-$421f", "");
  item("Controller 1 Data", string("0x", hex<4>(status.joy1)));
  item("Controller 2 Data", string("0x", hex<4>(status.joy2)));
  item("Controller 3 Data", string("0x", hex<4>(status.joy3)));
  item("Controller 4 Data", string("0x", hex<4>(status.joy4)));
  
  for(unsigned i = 0; i < 8; i++) {
    item(string("DMA Channel ", i), "");

    //$43x0
    item("Direction", channel[i].direction ? "B->A" : "A->B");
    item("Indirect", channel[i].indirect);
    item("Reverse Transfer", channel[i].reverse_transfer);
    item("Fixed Transfer", channel[i].fixed_transfer);
    item("Transfer Mode", (unsigned)channel[i].transfer_mode);

    //$43x1
    item("B-Bus Address", string("0x21", hex<2>(channel[i].dest_addr)));

    //$43x2-$43x3
    item("A-Bus Address", string("0x", hex<4>(channel[i].source_addr)));

    //$43x4
    item("A-Bus Bank", string("0x", hex<2>(channel[i].source_bank)));

    //$43x5-$43x6
    item("Transfer Size / Indirect Address", string("0x", hex<4>(channel[i].transfer_size)));

    //$43x7
    item("Indirect Bank", string("0x", hex<2>(channel[i].indirect_bank)));

    //$43x8-$43x9
    item("Table Address", string("0x", hex<4>(channel[i].hdma_addr)));

    //$43xa
    item("Line Counter", string("0x", hex<2>(channel[i].line_counter)));
  }

  #undef item
  return false;
}

unsigned CPUDebugger::getRegister(unsigned id) {
  switch ((Register)id) {
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

void CPUDebugger::setRegister(unsigned id, unsigned value) {
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

bool CPUDebugger::getFlag(unsigned id) {
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

void CPUDebugger::setFlag(unsigned id, bool value) {
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
