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
  
  sprintf(t, "%-14s ", nall::GBCPU::disassemble(addr, op, op0, op1)());
  strcat(s, t);
}

#endif
