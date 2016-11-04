debugvirtual alwaysinline uint8_t op_readpc() {
  return op_read(regs.pc++);
}

alwaysinline uint8_t op_readstack() {
  return op_read(0x0100 | ++regs.sp);
}

alwaysinline void op_writestack(uint8_t data) {
  op_write(0x0100 | regs.sp--, data);
}

template <bool usedebugger = true>
alwaysinline uint8_t op_readaddr(uint16_t addr) {
  if (usedebugger)
    // can be SMP::op_read or SMPDebugger::op_read
    return op_read(addr);
  else
    // always SMP::op_read
    return op_read_dummy(addr);
}

alwaysinline void op_writeaddr(uint16_t addr, uint8_t data) {
  op_write(addr, data);
}

template <bool usedebugger = true>
alwaysinline uint8_t op_readdp(uint8_t addr) {
  if (usedebugger)
    // can be SMP::op_read or SMPDebugger::op_read
    return op_read(((unsigned)regs.p.p << 8) + addr);
  else
    // always SMP::op_read
    return op_read_dummy(((unsigned)regs.p.p << 8) + addr);
}

alwaysinline void op_writedp(uint8_t addr, uint8_t data) {
  op_write(((unsigned)regs.p.p << 8) + addr, data);
}
