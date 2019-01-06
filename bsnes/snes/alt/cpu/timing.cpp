#ifdef CPU_CPP

void CPU::queue_event(unsigned id) {
  switch(id) {
    case QueueEvent::DramRefresh: return add_clocks(40);
    case QueueEvent::HdmaRun: return hdma_run();
    case QueueEvent::ControllerLatch: return ppu.latch_counters();
  }
}

void CPU::last_cycle() {
  if(status.irq_lock) {
    status.irq_lock = false;
    return;
  }

  if(status.nmi_transition) {
    regs.wai = false;
    status.nmi_transition = false;
    status.nmi_pending = true;
  }

  if(status.irq_transition || regs.irq) {
    regs.wai = false;
    status.irq_transition = false;
    status.irq_pending = !regs.p.i;
  }
}

void CPU::add_clocks(unsigned clocks) {
  if(status.hirq_enabled) {
    if(status.virq_enabled) {
      unsigned cpu_time = vcounter() * 1364 + hcounter();
      unsigned irq_time = status.virq_pos * 1364 + status.hirq_pos * 4;
      if(cpu_time > irq_time) irq_time += fieldlines() * 1364;
      bool irq_valid = status.irq_valid;
      status.irq_valid = cpu_time <= irq_time && cpu_time + clocks > irq_time;
      if(!irq_valid && status.irq_valid) status.irq_line = true;
    } else {
      unsigned irq_time = status.hirq_pos * 4;
      if(hcounter() > irq_time) irq_time += 1364;
      bool irq_valid = status.irq_valid;
      status.irq_valid = hcounter() <= irq_time && hcounter() + clocks > irq_time;
      if(!irq_valid && status.irq_valid) status.irq_line = true;
    }
    if(status.irq_line) status.irq_transition = true;
  } else if(status.virq_enabled) {
    bool irq_valid = status.irq_valid;
    status.irq_valid = vcounter() == status.virq_pos;
    if(!irq_valid && status.irq_valid) status.irq_line = true;
    if(status.irq_line) status.irq_transition = true;
  } else {
    status.irq_valid = false;
  }

  tick(clocks);
  queue.tick(clocks);
  step(clocks);
}

void CPU::scanline() {
  synchronize_smp();
  synchronize_ppu();
  synchronize_coprocessor();
  system.scanline();

  if(vcounter() == 0) hdma_init();

  queue.enqueue(534, QueueEvent::DramRefresh);

  if(vcounter() <= (ppu.overscan() == false ? 224 : 239)) {
    queue.enqueue(1104 + 8, QueueEvent::HdmaRun);
  }

  if(vcounter() == input.latchy) {
    queue.enqueue(input.latchx, QueueEvent::ControllerLatch);
  }

  bool nmi_valid = status.nmi_valid;
  status.nmi_valid = vcounter() >= (ppu.overscan() == false ? 225 : 240);
  if(!nmi_valid && status.nmi_valid) {
    status.nmi_line = true;
    if(status.nmi_enabled) status.nmi_transition = true;
  } else if(nmi_valid && !status.nmi_valid) {
    status.nmi_line = false;
  }

  if(status.auto_joypad_poll && vcounter() == (ppu.overscan() == false ? 227 : 242)) {
    input.poll();
    run_auto_joypad_poll();
  }
}

void CPU::run_auto_joypad_poll() {
  uint16 joy1 = 0, joy2 = 0, joy3 = 0, joy4 = 0;
  for(unsigned i = 0; i < 16; i++) {
    uint8 port0 = input.port_read(0);
    uint8 port1 = input.port_read(1);

    joy1 |= (port0 & 1) ? (0x8000 >> i) : 0;
    joy2 |= (port1 & 1) ? (0x8000 >> i) : 0;
    joy3 |= (port0 & 2) ? (0x8000 >> i) : 0;
    joy4 |= (port1 & 2) ? (0x8000 >> i) : 0;
  }

  status.joy1 = joy1;
  status.joy2 = joy2;
  status.joy3 = joy3;
  status.joy4 = joy4;
}

#endif
