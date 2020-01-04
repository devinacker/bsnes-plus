#ifdef SUPERGAMEBOY_CPP

#include <nall/snes/sgb.hpp>

SGBDebugger::SGBDebugger() {
  usage = new uint8_t[1 << 16](); // TODO
  cart_usage = 0;
  
  opcode_pc = 0;
}

SGBDebugger::~SGBDebugger() {
  delete[] usage;
//  delete[] cart_usage;
}

void SGBDebugger::init() {
  SuperGameBoy::init();
  if (opened()) {
    sgb_read_gb  = sym("sgb_read_gb");
    sgb_write_gb = sym("sgb_write_gb");
    sgb_get_reg  = sym("sgb_get_reg");
    sgb_set_reg  = sym("sgb_set_reg");
    sgb_get_flag = sym("sgb_get_flag");
    sgb_set_flag = sym("sgb_set_flag");
    
    // set up debugger callbacks
    function<void (void(*)(uint16_t))> stepcb = sym("sgb_callback_step");
    if (stepcb) stepcb(op_step);
    
    function<void (void(*)(uint16_t, uint8_t))> memcb;
    memcb = sym("sgb_callback_read");
    if (memcb) memcb(op_read);
    memcb = sym("sgb_callback_readpc");
    if (memcb) memcb(op_readpc);
    memcb = sym("sgb_callback_write");
    if (memcb) memcb(op_write);
  }
}

unsigned SGBDebugger::getRegister(unsigned id) {
  if (sgb_get_reg) switch (id) {
  case RegisterPC:
    return sgb_get_reg('P');
  case RegisterAF:
    return (sgb_get_reg('A') << 8) | sgb_get_reg('F');
  case RegisterBC:
    return (sgb_get_reg('B') << 8) | sgb_get_reg('C');
  case RegisterDE:
    return (sgb_get_reg('D') << 8) | sgb_get_reg('E');
  case RegisterHL:
    return (sgb_get_reg('H') << 8) | sgb_get_reg('L');
  case RegisterSP:
    return sgb_get_reg('S');
  }
  
  return 0;
}

void SGBDebugger::setRegister(unsigned id, unsigned value) {
  if (sgb_set_reg) switch (id) {
  case RegisterPC:
    sgb_set_reg('P', value);
    break;
  case RegisterAF:
    sgb_set_reg('A', value >> 8);
    sgb_set_reg('F', value);
    break;
  case RegisterBC:
    sgb_set_reg('B', value >> 8);
    sgb_set_reg('C', value);
    break;
  case RegisterDE:
    sgb_set_reg('D', value >> 8);
    sgb_set_reg('E', value);
    break;
  case RegisterHL:
    sgb_set_reg('H', value >> 8);
    sgb_set_reg('L', value);
    break;
  case RegisterSP:
    sgb_set_reg('S', value);
    break;
  }
}

bool SGBDebugger::getFlag(unsigned id) {
  if (sgb_get_flag) switch (id) {
  case FlagZ: return sgb_get_flag('Z');
  case FlagN: return sgb_get_flag('N');
  case FlagH: return sgb_get_flag('H');
  case FlagC: return sgb_get_flag('C');
  }
  
  return false;
}

void SGBDebugger::setFlag(unsigned id, bool value) {
  if (sgb_set_flag) switch (id) {
  case FlagZ: sgb_set_flag('Z', value); break;
  case FlagN: sgb_set_flag('N', value); break;
  case FlagH: sgb_set_flag('H', value); break;
  case FlagC: sgb_set_flag('C', value); break;
  }
}

uint8_t SGBDebugger::read_gb(uint16_t addr) {
  if (sgb_read_gb) return sgb_read_gb(addr);
  return 0;
}

void SGBDebugger::write_gb(uint16_t addr, uint8_t data) {
  if (sgb_write_gb) sgb_write_gb(addr, data);
}

void SGBDebugger::op_step(uint16_t pc) {
  supergameboy.usage[pc] |= UsageOpcode;
  supergameboy.opcode_pc = pc;
  
  if(debugger.step_sgb &&
     (debugger.step_type == Debugger::StepType::StepInto)) {
    debugger.break_event = Debugger::BreakEvent::SGBStep;
    debugger.step_type = Debugger::StepType::None;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::Source::SGBBus, Debugger::Breakpoint::Mode::Exec, pc, 0x00);
  }
  
  if (supergameboy.step_event) supergameboy.step_event();
}

void SGBDebugger::op_read(uint16_t addr, uint8_t data) {
  supergameboy.usage[addr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SGBBus, Debugger::Breakpoint::Mode::Read, addr, data);
}

void SGBDebugger::op_readpc(uint16_t pc, uint8_t data) {
  supergameboy.usage[pc] |= UsageExec;
}

void SGBDebugger::op_write(uint16_t addr, uint8_t data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SGBBus, Debugger::Breakpoint::Mode::Write, addr, data);
  supergameboy.usage[addr] |= UsageWrite;
  supergameboy.usage[addr] &= ~UsageExec;
}

void SGBDebugger::disassemble_opcode(char *output, uint16_t addr) {
  char t[256];
  char *s = output;

  if (!sgb_read_gb) return;

  sprintf(s, "..%.4x ", addr);

  uint8 op  = sgb_read_gb(addr);
  uint8 op0 = sgb_read_gb(addr + 1);
  uint8 op1 = sgb_read_gb(addr + 2);
  
  sprintf(t, "%-23s ", nall::GBCPU::disassemble(addr, op, op0, op1)());
  strcat(s, t);
  
  uint16_t af = getRegister(RegisterAF);
  uint16_t bc = getRegister(RegisterBC);
  uint16_t de = getRegister(RegisterDE);
  uint16_t hl = getRegister(RegisterHL);
  uint16_t sp = getRegister(RegisterSP);
  sprintf(t, "AF:%.4x BC:%.4x DE:%.4x HL:%.4x SP:%.4x ", af, bc, de, hl, sp);
  strcat(s, t);
  
  sprintf(t, "%c%c%c%c ",
    (af & 0x80) ? 'Z' : '.', (af & 0x40) ? 'N' : '.',
    (af & 0x20) ? 'H' : '.', (af & 0x10) ? 'C' : '.');
  strcat(s, t);
}

#endif
