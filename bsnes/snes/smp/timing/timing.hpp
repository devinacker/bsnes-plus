template<unsigned timer_frequency>
class sSMPTimer {
public:
  uint8 stage0_ticks;
  uint8 stage1_ticks;
  uint8 stage2_ticks;
  uint8 stage3_ticks;
  bool current_line;
  bool enabled;
  uint8 target;

  void step(unsigned clocks);
  void sync_stage1();
};

sSMPTimer<128> t0;
sSMPTimer<128> t1;
sSMPTimer< 16> t2;

alwaysinline void wait(uint16 addr);
alwaysinline void add_clocks(unsigned clocks);
alwaysinline void step_timers(unsigned clocks);
