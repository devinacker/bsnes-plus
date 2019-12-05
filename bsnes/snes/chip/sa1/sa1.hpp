#include "bus/bus.hpp"

class SA1 : public Coprocessor, public CPUcore, public MMIO {
public:
  #include "dma/dma.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"

  struct Status {
    uint8 tick_counter;

    bool interrupt_pending;
    uint16 interrupt_vector;

    uint16 scanlines;
    uint16 vcounter;
    uint16 hcounter;
  } status;

  static void Enter();
  void enter();
  debugvirtual void interrupt(uint16 vector);
  void tick();
  
  // used by the SA-1 debugger prior to executing instructions
  debugvirtual void op_step() {};

  alwaysinline void trigger_irq();
  alwaysinline void last_cycle();
  alwaysinline bool interrupt_pending();

  void init();
  void enable();
  void power();
  void reset();

  void serialize(serializer&);
  SA1();
};

#if defined(DEBUGGER)
  #include "debugger/debugger.hpp"
  extern SA1Debugger sa1;
  extern VBRBus vbrbus;
  
  extern CPUAnalyst sa1Analyst;
#else
  extern SA1 sa1;
#endif
extern SA1Bus sa1bus;
