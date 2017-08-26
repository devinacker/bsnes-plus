#include <snes.hpp>

#define CX4_CPP
namespace SNES {

#include "bus.cpp"
#include "memory.cpp"
#include "registers.cpp"
#include "instructions.cpp"
#include "data.cpp"
#include "serialization.cpp"

Cx4 cx4;

void Cx4::Enter() { cx4.enter(); }

void Cx4::enter() {
  while (true) {
    if (scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    if (mmio.dma) {
      for (unsigned n = 0; n < mmio.dmaLength; n++) {
        cx4bus.write(mmio.dmaTarget + n, cx4bus.read(mmio.dmaSource + n));
        step(2);
        synchronize_cpu();
      }
      mmio.dma = false;
    }

    if (!regs.halt) {
      uint24 addr = mmio.programOffset + (regs.pc * 2);
      opcode  = cx4bus.read(addr++) << 0;
      opcode |= cx4bus.read(addr++) << 8;
      regs.pc = (regs.pc & 0xffff00) | ((regs.pc + 1) & 0x0000ff);
      instruction();
    }
    step(1);
    synchronize_cpu();
  }
}

void Cx4::init() {
}

void Cx4::enable() {
}

void Cx4::power() {
  cx4bus.init();
  reset();
}

void Cx4::reset() {
  create(Cx4::Enter, frequency);
  
  memset(dataRAM, 0, sizeof(dataRAM));
  
  regs.halt = true;

  regs.n = 0;
  regs.z = 0;
  regs.c = 0;
  
  mmio.dma = false;

  mmio.dmaSource = 0x000000;
  mmio.dmaLength = 0x0000;
  mmio.dmaTarget = 0x000000;
  mmio.r1f48 = 0x00;
  mmio.programOffset = 0x000000;
  mmio.r1f4c = 0x00;
  mmio.pageNumber = 0x0000;
  mmio.programCounter = 0x00;
  mmio.r1f50 = 0x33;
  mmio.r1f51 = 0x00;
  mmio.r1f52 = 0x01;

}

}
