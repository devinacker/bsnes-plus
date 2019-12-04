#include "SPC_DSP.h"

class DSP : public Processor {
public:
  enum : bool { Threaded = false };
  enum : bool { SupportsChannelEnable = true };

  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_smp();

  bool mute();
  uint8 read(uint8 addr);
  void write(uint8 addr, uint8 data);
  void load(uint8 const regs [SPC_DSP::register_count]);

  void enter();
  void power();
  void reset();

  void channel_enable(unsigned channel, bool enable);
  bool is_channel_enabled(unsigned channel);

  void serialize(serializer&);
  DSP();

private:
  void update_channels();

  SPC_DSP spc_dsp;
  int16 samplebuffer[8192];
  bool channel_enabled[8];
};

#if defined(DEBUGGER)
  #include "../../dsp/debugger/debugger.hpp"
  extern DSPDebugger dsp;
#else
  extern DSP dsp;
#endif
