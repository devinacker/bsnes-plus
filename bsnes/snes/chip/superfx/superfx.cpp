#include <snes.hpp>

#define SUPERFX_CPP
namespace SNES {

#include "serialization.cpp"
#include "bus/bus.cpp"
#include "core/core.cpp"
#include "memory/memory.cpp"
#include "mmio/mmio.cpp"
#include "timing/timing.cpp"
#include "disasm/disasm.cpp"

#if defined(DEBUGGER)
  #include "debugger/debugger.cpp"
  SFXDebugger superfx;
#else
  SuperFX superfx;
#endif

void SuperFX::Enter() { superfx.enter(); }

void SuperFX::enter() {
  while(true) {
    scheduler.synchronize();

    if(regs.sfr.g == 0) {
      add_clocks(6);
      continue;
    }

    op_step();

    op_exec(peekpipe());
    if(r15_modified == false) regs.r[15]++;
  }
}

void SuperFX::init() {
  regs.r[14].on_modify = { &SuperFX::r14_modify, this };
  regs.r[15].on_modify = { &SuperFX::r15_modify, this };
}

void SuperFX::enable() {
}

void SuperFX::power() {
  regs.clsr  = 0;
  reset();
}

void SuperFX::reset() {
  create(SuperFX::Enter, system.cpu_frequency());
  superfxbus.init();

  for(unsigned n = 0; n < 16; n++) regs.r[n] = 0x0000;
  regs.sfr   = 0x0000;
  regs.pbr   = 0x00;
  regs.rombr = 0x00;
  regs.rambr = 0;
  regs.cbr   = 0x0000;
  regs.scbr  = 0x00;
  regs.scmr  = 0x00;
  regs.colr  = 0x00;
  regs.por   = 0x00;
  regs.bramr = 0;
  regs.vcr   = 0x04;
  regs.cfgr  = 0x00;
  regs.clsr  = 0;
  regs.pipeline = 0x01;  //nop
  regs.ramaddr = 0x0000;
  regs.reset();

  disassemble_lastregs = 0;

  memory_reset();
  timing_reset();
}

}
