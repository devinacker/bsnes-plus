#ifdef SMP_CPP

//DSP clock (~24576khz) / 12 (~2048khz) is fed into the SMP
//from here, the wait states value is really a clock divider of {2, 4, 8, 16}
//due to an unknown hardware issue, clock dividers of 8 and 16 are glitchy
//the SMP ends up consuming 10 and 20 clocks per opcode cycle instead
//this causes unpredictable behavior on real hardware
//sometimes the SMP will run far slower than expected
//other times (and more likely), the SMP will deadlock until the system is reset
//the timers are not affected by this and advance by their expected values
void SMP::wait(uint16 addr) {
  static const unsigned cycleWaitStates[4] = {2, 4, 10, 20};
  static const unsigned timerWaitStates[4] = {2, 4,  8, 16};

  unsigned waitStates = status.external_speed;
  if((addr & 0xfff0) == 0x00f0) 
    waitStates = status.internal_speed;  //IO registers + idle cycles
  else if(addr >= 0xffc0 && status.iplrom_enabled) 
    waitStates = status.internal_speed;  //IPLROM

  add_clocks(12 * cycleWaitStates[waitStates]);
  step_timers(timerWaitStates[waitStates]);
}

void SMP::add_clocks(unsigned clocks) {
  step(clocks);
  synchronize_dsp();

  //forcefully sync S-SMP to S-CPU in case chips are not communicating
  //sync if S-SMP is more than 24 samples ahead of S-CPU
  if(clock > +(768 * 24 * (int64)24000000)) synchronize_cpu();
}

void SMP::step_timers(unsigned clocks) {
  t0.step(clocks);
  t1.step(clocks);
  t2.step(clocks);
}

template<unsigned timer_frequency>
void SMP::sSMPTimer<timer_frequency>::step(unsigned clocks) {
  //stage 0 increment
  stage0_ticks += clocks;
  if(stage0_ticks < timer_frequency) return;
  stage0_ticks -= timer_frequency;

  //stage 1 increment
  stage1_ticks ^= 1;
  sync_stage1();
}

template<unsigned frequency>
void SMP::sSMPTimer<frequency>::sync_stage1() {
  bool new_line = stage1_ticks;
  if(smp.status.timers_enabled == false) new_line = false;
  if(smp.status.timers_disabled == true) new_line = false;

  bool old_line = current_line;
  current_line = new_line;
  if(old_line != 1 || new_line != 0) return;  //only pulse on 1->0 transition

  //stage 2 increment
  if(enabled == false) return;
  stage2_ticks++;
  if(stage2_ticks != target) return;

  //stage 3 increment
  stage2_ticks = 0;
  stage3_ticks++;
  stage3_ticks &= 15;
}

#endif
