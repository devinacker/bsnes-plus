#ifdef CX4_CPP

void Cx4::serialize(serializer &s) {
  Processor::serialize(s);

  for(auto& n : stack) s.integer(n);
  s.integer(opcode);

  s.integer(regs.halt);
  s.integer(regs.cachePage);
  s.integer(regs.irqPending);
  
  s.integer(regs.rwbusaddr);
  s.integer(regs.rwbustime);
  s.integer(regs.writebus);
  s.integer(regs.writebusdata);

  s.integer(regs.pc);
  s.integer(regs.p);
  s.integer(regs.n);
  s.integer(regs.z);
  s.integer(regs.v);
  s.integer(regs.c);

  s.integer(regs.a);
  s.integer(regs.acch);
  s.integer(regs.accl);
  s.integer(regs.busdata);
  s.integer(regs.romdata);
  s.integer(regs.ramdata);
  s.integer(regs.busaddr);
  s.integer(regs.ramaddr);
  for(auto& n : regs.gpr) s.integer(n);
  s.integer(regs.mdr);

  s.integer(mmio.dma);
  s.integer(mmio.suspend);
  s.integer(mmio.cacheLoading);
  s.integer(mmio.dmaSource);
  s.integer(mmio.dmaLength);
  s.integer(mmio.dmaTarget);
  s.integer(mmio.cachePreload);
  s.integer(mmio.programOffset);
  s.integer(mmio.pageNumber);
  s.integer(mmio.programCounter);
  s.integer(mmio.romSpeed);
  s.integer(mmio.ramSpeed);
  s.integer(mmio.irqDisable);
  s.integer(mmio.r1f52);
  s.integer(mmio.suspendCycles);
  s.array(mmio.vector);
  
  for(auto& page : cache) {
    s.integer(page.lock);
    s.integer(page.pageNumber);
    s.array(page.data);
  }
}

#endif
