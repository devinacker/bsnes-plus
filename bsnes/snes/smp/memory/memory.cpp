#ifdef SMP_CPP

alwaysinline uint8 SMP::ram_read(uint16 addr) {
  if(addr >= 0xffc0 && status.iplrom_enabled) return iplrom[addr & 0x3f];
  if(status.ram_disabled) return 0x5a;  //0xff on mini-SNES
  return memory::apuram[addr];
}

alwaysinline void SMP::ram_write(uint16 addr, uint8 data) {
  //writes to $ffc0-$ffff always go to apuram, even if the iplrom is enabled
  if(status.ram_writable && !status.ram_disabled) memory::apuram[addr] = data;
}

uint8 SMP::op_debugread(uint16 addr) {
  return op_busread(addr);
}

alwaysinline uint8 SMP::op_busread(uint16 addr) {
  uint8 r;
  if((addr & 0xfff0) == 0x00f0) {  //00f0-00ff
    switch(addr) {
      case 0xf0: {  //TEST -- write-only register
        r = 0x00;
      } break;

      case 0xf1: {  //CONTROL -- write-only register
        r = 0x00;
      } break;

      case 0xf2: {  //DSPADDR
        r = status.dsp_addr;
      } break;

      case 0xf3: {  //DSPDATA
        //0x80-0xff are read-only mirrors of 0x00-0x7f
        r = dsp.read(status.dsp_addr & 0x7f);
      } break;

      case 0xf4:    //CPUIO0
      case 0xf5:    //CPUIO1
      case 0xf6:    //CPUIO2
      case 0xf7: {  //CPUIO3
        if (!Memory::debugger_access())
          synchronize_cpu();
        r = port.cpu_to_smp[addr & 3];
      } break;

      case 0xf8:    //PORT4
      case 0xf9: {  //PORT5
        r = port.aux[addr & 1];
      } break;

      case 0xfa:    //T0TARGET
      case 0xfb:    //T1TARGET
      case 0xfc: {  //T2TARGET -- write-only registers
        r = 0x00;
      } break;

      case 0xfd: {  //T0OUT -- 4-bit counter value
        r = t0.stage3_ticks & 15;
        if (!Memory::debugger_access())
          t0.stage3_ticks = 0;
      } break;

      case 0xfe: {  //T1OUT -- 4-bit counter value
        r = t1.stage3_ticks & 15;
        if (!Memory::debugger_access())
          t1.stage3_ticks = 0;
      } break;

      case 0xff: {  //T2OUT -- 4-bit counter value
        r = t2.stage3_ticks & 15;
        if (!Memory::debugger_access())
          t2.stage3_ticks = 0;
      } break;
    }
  } else {
    r = ram_read(addr);
  }

  return r;
}

alwaysinline void SMP::op_buswrite(uint16 addr, uint8 data) {
  if((addr & 0xfff0) == 0x00f0) {  //$00f0-00ff
    switch(addr) {
      case 0xf0: {  //TEST
        if(regs.p.p) break;  //writes only valid when P flag is clear

        status.internal_speed  = (data >> 6) & 3;
        status.external_speed  = (data >> 4) & 3;
        status.timers_enabled  = data & 0x08;
        status.ram_disabled    = data & 0x04;
        status.ram_writable    = data & 0x02;
        status.timers_disabled = data & 0x01;

        t0.sync_stage1();
        t1.sync_stage1();
        t2.sync_stage1();
      } break;

      case 0xf1: {  //CONTROL
        status.iplrom_enabled = data & 0x80;

        if(data & 0x30) {
          //one-time clearing of APU port read registers
          synchronize_cpu();
          if(data & 0x20) {
            port.cpu_to_smp[2] = 0;
            port.cpu_to_smp[3] = 0;
          }
          if(data & 0x10) {
            port.cpu_to_smp[0] = 0;
            port.cpu_to_smp[1] = 0;
          }
        }

        //0->1 transistion resets timers
        if(t2.enabled == false && (data & 0x04)) {
          t2.stage2_ticks = 0;
          t2.stage3_ticks = 0;
        }
        t2.enabled = data & 0x04;

        if(t1.enabled == false && (data & 0x02)) {
          t1.stage2_ticks = 0;
          t1.stage3_ticks = 0;
        }
        t1.enabled = data & 0x02;

        if(t0.enabled == false && (data & 0x01)) {
          t0.stage2_ticks = 0;
          t0.stage3_ticks = 0;
        }
        t0.enabled = data & 0x01;
      } break;

      case 0xf2: {  //DSPADDR
        status.dsp_addr = data;
      } break;

      case 0xf3: {  //DSPDATA
        //0x80-0xff are read-only mirrors of 0x00-0x7f
        if(!(status.dsp_addr & 0x80)) {
          dsp.write(status.dsp_addr & 0x7f, data);
        }
        
        if (dump_spc && status.dsp_addr == 0x4c /* r_kon */ && data) {
          save_spc_dump();
        }
      } break;

      case 0xf4:    //CPUIO0
      case 0xf5:    //CPUIO1
      case 0xf6:    //CPUIO2
      case 0xf7: {  //CPUIO3
        synchronize_cpu();
        port.smp_to_cpu[addr & 3] = data;
      } break;

      case 0xf8:    //PORT4
      case 0xf9: {  //PORT5
        port.aux[addr & 1] = data;
      } break;

      case 0xfa: {  //T0TARGET
        t0.target = data;
      } break;

      case 0xfb: {  //T1TARGET
        t1.target = data;
      } break;

      case 0xfc: {  //T2TARGET
        t2.target = data;
      } break;

      case 0xfd:    //T0OUT
      case 0xfe:    //T1OUT
      case 0xff: {  //T2OUT -- read-only registers
      } break;
    }
  }

  //all writes, even to MMIO registers, appear on bus
  ram_write(addr, data);
}

void SMP::op_io() {
  wait(0x00f0); // idle cycles use same timing as I/O registers
}

uint8 SMP::op_read(uint16 addr) {
  wait(addr);
  return op_busread(addr);
}

void SMP::op_write(uint16 addr, uint8 data) {
  wait(addr);
  op_buswrite(addr, data);
}

#endif
