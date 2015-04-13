class SMP : public Processor, public SMPcore, public MMIO {
public:
  enum : bool { Threaded = true };
  alwaysinline void step(unsigned clocks);
  alwaysinline void synchronize_cpu();
  alwaysinline void synchronize_dsp();

  void enter();
  void power();
  void reset();

  void serialize(serializer&);
  SMP();
  ~SMP();

  static const uint8 iplrom[64];

private:
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"

  struct {
    //timing
    unsigned clock_counter;
    unsigned dsp_counter;
    unsigned timer_step;

    //$00f0
    uint8 clock_speed;
    uint8 timer_speed;
    bool timers_enabled;
    bool ram_disabled;
    bool ram_writable;
    bool timers_disabled;

    //$00f1
    bool iplrom_enabled;

    //$00f2
    uint8 dsp_addr;
  } status;

  struct {
    //$00f4-$00f7
    uint8 cpu_to_smp[4];
    uint8 smp_to_cpu[4];

    //$00f8-$00f9
    uint8 aux[2];
  } port;

  static void Enter();
  debugvirtual void op_step();

  friend class SMPcore;
  friend class SMPDebugger;
};

#if defined(DEBUGGER)
  #include "debugger/debugger.hpp"
  extern SMPDebugger smp;
#else
  extern SMP smp;
#endif
