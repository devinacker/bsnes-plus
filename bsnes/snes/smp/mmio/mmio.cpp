#ifdef SMP_CPP

uint8 SMP::mmio_read(unsigned addr) {
  cpu.synchronize_smp();
  return port.smp_to_cpu[addr & 3];
}

void SMP::mmio_write(unsigned addr, uint8 data) {
  cpu.synchronize_smp();
  port.cpu_to_smp[addr & 3] = data;
}

#endif
