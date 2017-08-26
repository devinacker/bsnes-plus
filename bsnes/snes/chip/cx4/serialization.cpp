#ifdef CX4_CPP

void Cx4::serialize(serializer &s) {
  Processor::serialize(s);

  for(auto& n : stack) s.integer(n);
  s.integer(opcode);

  s.integer(regs.halt);

  s.integer(regs.pc);
  s.integer(regs.p);
  s.integer(regs.n);
  s.integer(regs.z);
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

  s.integer(mmio.dma);
  s.integer(mmio.dmaSource);
  s.integer(mmio.dmaLength);
  s.integer(mmio.dmaTarget);
  s.integer(mmio.r1f48);
  s.integer(mmio.programOffset);
  s.integer(mmio.r1f4c);
  s.integer(mmio.pageNumber);
  s.integer(mmio.programCounter);
  s.integer(mmio.r1f50);
  s.integer(mmio.r1f51);
  s.integer(mmio.r1f52);
  s.array(mmio.vector);
}

#endif
