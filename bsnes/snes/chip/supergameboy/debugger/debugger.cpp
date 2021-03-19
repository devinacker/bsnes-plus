#ifdef SUPERGAMEBOY_CPP

#include <nall/snes/sgb.hpp>
using namespace nall;

#include "disassembler.cpp"

SGBDebugger::SGBDebugger() {
  usage_ = new uint8_t[1 << 24]();
  cart_usage = 0; // TODO
  
  opcode_pc = 0;
}

SGBDebugger::~SGBDebugger() {
  delete[] usage_;
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

    sgb_addr_with_bank = sym("sgb_addr_with_bank");

    // set up debugger callbacks
    function<void (void(*)(uint32_t))> stepcb;
    stepcb = sym("sgb_callback_step");
    if (stepcb) stepcb(op_step);
    stepcb = sym("sgb_callback_call");
    if (stepcb) stepcb(op_call);
    stepcb = sym("sgb_callback_ret");
    if (stepcb) stepcb(op_ret);
    stepcb = sym("sgb_callback_irq");
    if (stepcb) stepcb(op_irq);
    
    function<void (void(*)(uint32_t, uint8_t))> memcb;
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

uint8_t& SGBDebugger::usage(uint16_t addr) {
  if (sgb_addr_with_bank) return usage_[sgb_addr_with_bank(addr)];
  return usage_[addr];
}

void SGBDebugger::op_call(uint32_t addr) {
  if (debugger.step_sgb) {
    debugger.call_count++;
  }
}

void SGBDebugger::op_irq(uint32_t addr) {
  if (debugger.step_sgb) {
    debugger.call_count++;
    
    if (debugger.step_type == Debugger::StepType::StepToIRQ) {
      // break on next instruction after interrupt
      debugger.step_type = Debugger::StepType::StepInto;
    }
  }
}

void SGBDebugger::op_ret(uint32_t addr) {
  if (debugger.step_sgb) {
    debugger.call_count--;
  }
}

void SGBDebugger::op_step(uint32_t pc) {
  supergameboy.usage_[pc] |= UsageOpcode;
  supergameboy.opcode_pc = pc;
  
  if(debugger.step_sgb &&
     (debugger.step_type == Debugger::StepType::StepInto ||
       (debugger.step_type >= Debugger::StepType::StepOver && debugger.call_count < 0))) {
    debugger.break_event = Debugger::BreakEvent::SGBStep;
    debugger.step_type = Debugger::StepType::None;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
  } else {
    debugger.breakpoint_test(Debugger::Breakpoint::Source::SGBBus, Debugger::Breakpoint::Mode::Exec, pc, 0x00);
  }
  
  if (supergameboy.step_event) supergameboy.step_event();
  
  // adjust call count if this is a call or return
  // (or if we're stepping over and no call occurred)
  if (debugger.step_sgb) {
    if (debugger.step_over_new && debugger.call_count == 0) {
      debugger.call_count = -1;
      debugger.step_over_new = false;
    }
  }
}

void SGBDebugger::op_read(uint32_t addr, uint8_t data) {
  supergameboy.usage_[addr] |= UsageRead;
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SGBBus, Debugger::Breakpoint::Mode::Read, addr, data);
}

void SGBDebugger::op_readpc(uint32_t pc, uint8_t data) {
  supergameboy.usage_[pc] |= UsageExec;
}

void SGBDebugger::op_write(uint32_t addr, uint8_t data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::SGBBus, Debugger::Breakpoint::Mode::Write, addr, data);
  supergameboy.usage_[addr] |= UsageWrite;
  supergameboy.usage_[addr] &= ~UsageExec;
}

#endif
