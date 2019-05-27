#ifdef SA1_CPP

uint8 SA1Debugger::disassembler_read(uint32 addr)
{
  debugger.bus_access = true;
  uint8 data = sa1bus.read(addr);
  debugger.bus_access = false;
  return data;
}

void SA1Debugger::interrupt(uint16 vector) {
  SA1::interrupt(vector);
  
  if (debugger.step_sa1) {
    debugger.call_count++;
    
    if ((debugger.step_type == Debugger::StepType::StepToNMI && (vector & 0xf) == 0xa)
        || (debugger.step_type == Debugger::StepType::StepToIRQ && (vector & 0xf) == 0xe)) {
      // break on next instruction after interrupt
      debugger.step_type = Debugger::StepType::StepInto;
    }
  }
}

void SA1Debugger::op_step() {
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

    if (debugger.break_on_wdm || debugger.break_on_brk) {
      uint8 opcode = disassembler_read(opcode_pc);
      if ((opcode == 0x42 && debugger.break_on_wdm) || (opcode == 0x00 && debugger.break_on_brk)) {
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
  if (debugger.step_sa1) {
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
  item("SA-1 open bus", string("0x", hex<2>(regs.mdr)));
  
  // $2200
  item("$2200", "");
  item("SA-1 Ready", mmio.sa1_rdyb);
  item("SA-1 Reset", mmio.sa1_resb);
  item("S-CPU to SA-1 Message", string("0x", hex<1>(mmio.smeg)));
  
  // $2201
  item("$2201", "");
  item("S-CPU IRQ Enable", mmio.cpu_irqen);
  item("Char DMA IRQ Enable", mmio.chdma_irqen);
  
  // $2203-2208
  item("$2203-$2204", "");
  item("SA-1 Reset Vector", string("0x", hex<4>(mmio.crv)));
  item("$2205-$2206", "");
  item("SA-1 NMI Vector", string("0x", hex<4>(mmio.cnv)));
  item("$2207-$2208", "");
  item("SA-1 IRQ Vector", string("0x", hex<4>(mmio.civ)));
  
  // $2209
  item("$2209", "");
  item("S-CPU IRQ Vector Enable", mmio.cpu_ivsw);
  item("S-CPU NMI Vector Enable", mmio.cpu_nvsw);
  item("SA-1 to S-CPU Message", string("0x", hex<1>(mmio.cmeg)));
  
  // $220a
  item("$220a", "");
  item("SA-1 IRQ Enable", mmio.sa1_irqen);
  item("SA-1 Timer IRQ Enable", mmio.timer_irqen);
  item("SA-1 DMA IRQ Enable", mmio.dma_irqen);
  item("SA-1 NMI Enable", mmio.sa1_nmien);
  
  // $220c-220f
  item("$220c-$220d", "");
  item("S-CPU NMI Vector", string("0x", hex<4>(mmio.snv)));
  item("$220e-$220f", "");
  item("S-CPU IRQ Vector", string("0x", hex<4>(mmio.siv)));
  
  // $2210
  item("$2210", "");
  item("Timer Type", mmio.hvselb ? "Linear" : "H/V");
  item("V-Count IRQ Enable", mmio.ven);
  item("H-Count IRQ Enable", mmio.hen);
  
  // $2212-2213
  item("$2212-$2213", "");
  item("H-Count", string("0x", hex<4>(mmio.hcnt)));

  // $2214-2215
  item("$2214-$2215", "");
  item("V-Count", string("0x", hex<4>(mmio.vcnt)));
  
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
  item("Bank F ($A0-$BF)", string("0x", hex<1>(mmio.fb), " (0x", hex<6>(mmio.fb << 20), ")"));
  
  // $2224-2225
  item("$2224", "");
  item("S-CPU BW-RAM Bank", string("0x", hex<1>(mmio.sbm), " (0x", hex<5>(mmio.sbm << 13), ")"));
  
  item("$2225", "");
  item("SA-1 BW-RAM Bitmap", mmio.sw46);
  item("SA-1 BW-RAM Bank", string("0x", hex<1>(mmio.cbm), " (0x", hex<5>(mmio.cbm << 13), ")"));
  
  // $2226-222a
  item("$2226", "");
  item("S-CPU BW-RAM Write Enable", mmio.swen);
  
  item("$2227", "");
  item("SA-1 BW-RAM Write Enable", mmio.cwen);
  
  item("$2228", "");
  item("BW-RAM Write Protect Size", string("0x", hex<3>(mmio.bwp << 8)));
  
  item("$2229", "");
  for (unsigned i = 0; i < 8; i++) {
    item(string("S-CPU IRAM Write Protect 0x", hex<4>(0x3000 + (i<<8))), (bool)((mmio.siwp>>i) & 1));
  }
  
  item("$222a", "");
  for (unsigned i = 0; i < 8; i++) {
    item(string("SA-1 IRAM Write Protect 0x", hex<4>(0x3000 + (i<<8))), (bool)((mmio.ciwp>>i) & 1));
  }
  
  // $2230-2231
  item("$2230", "");
  string sd;
  
  switch (mmio.sd) {
  case 0: sd = "ROM"; break;
  case 1: sd = "BW-RAM"; break;
  case 2: sd = "IRAM"; break;
  default: sd = "Invalid"; break;
  }
  
  item("DMA Enable", mmio.dmaen);
  item("DMA Priority", mmio.dprio);
  item("DMA Char Conversion Enable", mmio.cden);
  item("DMA Char Conversion Type", mmio.cdsel);
  item("DMA Destination", mmio.dd ? "BW-RAM" : "IRAM");
  item("DMA Source", sd);
  
  item("$2231", "")
  string cb;
  
  switch (mmio.dmacb) {
  case 0: cb = "8 bpp"; break;
  case 1: cb = "4 bpp"; break;
  case 2: cb = "2 bpp"; break;
  default: cb = "Invalid"; break;
  }
  
  item("DMA Size", string(1<<mmio.dmasize, " tile(s)"));
  item("DMA Bit Depth", cb); 
  
  // $2232-2234
  item("$2232-$2234", "");
  item("DMA Source Address", string("0x", hex<6>(mmio.dsa)));
  
  // $2235-2237
  item("$2235-$2237", "");
  item("DMA Destination Address", string("0x", hex<6>(mmio.dda)));
  
  // $2238-2239
  item("$2238-$2239", "");
  item("DMA Terminal Counter", string("0x", hex<4>(mmio.dtc)));
  
  // $223f
  item("$223f", "");
  item("BW-RAM Bitmap Format", mmio.bbf ? "2 bpp" : "4 bpp");
  
  // $2250-2254
  item("$2250", "");
  item("Arithmetic Operation", mmio.acm ? "Cumulative sum" : (mmio.md ? "Division" : "Multiplication"));
  
  item("$2251-$2252", "");
  item("Multiplicand / Dividend", string((int16)mmio.ma, " (0x", hex<4>(mmio.ma), ")"));
  
  item("$2253-$2254", "");
  item("Multiplier", string((int16)mmio.mb, " (0x", hex<4>(mmio.mb), ")"));
  item("Divisor", string(mmio.mb, " (0x", hex<4>(mmio.mb), ")"));
  
  // $2258
  item("$2258", "");
  item("Variable-Length Bit Auto Increment", mmio.hl);
  item("Variable-Length Bit Length", (unsigned)mmio.vb);
  
  // $2259-225b
  item("$2259-$225b", "");
  item("Variable-Length Bit Start Address", mmio.va);
  
  // $2300
  item("$2300", "");
  item("S-CPU IRQ", mmio.cpu_irqfl);
  item("Char DMA IRQ", mmio.chdma_irqfl);
  
  // $2301
  item("$2301", "");
  item("SA-1 IRQ", mmio.sa1_irqfl);
  item("SA-1 Timer IRQ", mmio.timer_irqfl);
  item("SA-1 DMA IRQ", mmio.dma_irqfl);
  item("SA-1 NMI", mmio.sa1_nmifl);
  
  // $2302-2303
  item("$2302-$2303", "");
  item("H-Count Read", string("0x", hex<4>(mmio.hcr)));

  // $2304-2305
  item("$2304-$2305", "");
  item("V-Count Read", string("0x", hex<4>(mmio.vcr)));
  
  // $2306-230b
  item("$2306-$230b", "");
  int32  mult = (int32)(uint32)mmio.mr;
  int16  div  = (int16)(uint16)mmio.mr;
  uint16 divr = (mmio.mr >> 16) & 0xffff;
  item("Multiplication Result", string(mult, " (0x", hex<8>((uint32)mmio.mr), ")"));
  item("Division Result", string(div, " (0x", hex<4>((uint16)mmio.mr), ")"));
  item("Division Remainder", string(divr, " (0x", hex<4>(divr), ")"));
  item("Cumulative Sum", string("0x", hex<10>(mmio.mr & ((1ULL << 40) - 1))));
  item("Arithmetic Overflow", mmio.overflow);
  
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
