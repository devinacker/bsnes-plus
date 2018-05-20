#ifdef CPU_CPP

void CPU::joypad_edge() {
  if(vcounter() >= (ppu.overscan() == false ? 225 : 240)) {
    //cache enable state at first iteration
    if(status.auto_joypad_counter == 0) 
      status.auto_joypad_latch = status.auto_joypad_poll;
    status.auto_joypad_active = status.auto_joypad_counter <= 15;

    if(status.auto_joypad_active && status.auto_joypad_latch) {
      if(status.auto_joypad_counter == 0) {
        input.poll();

        //shift registers are cleared at start of auto joypad polling
        status.joy1 = 0;
        status.joy2 = 0;
        status.joy3 = 0;
        status.joy4 = 0;
      }

      unsigned port0 = input.port_read(0) & 3;
      unsigned port1 = input.port_read(1) & 3;

      status.joy1 = (status.joy1 << 1) | (port0 & 1);
      status.joy2 = (status.joy2 << 1) | (port1 & 1);
      status.joy3 = (status.joy3 << 1) | (port0 >> 1);
      status.joy4 = (status.joy4 << 1) | (port1 >> 1);
    }

    status.auto_joypad_counter++;
  }
}

#endif
