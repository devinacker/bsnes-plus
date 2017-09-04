#include "bus/bus.hpp"

class SuperFX : public Coprocessor, public MMIO {
public:
  #include "core/core.hpp"
  #include "memory/memory.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"
  #include "disasm/disasm.hpp"

  static void Enter();
  void enter();
  void init();
  void enable();
  void power();
  debugvirtual void reset();
  void serialize(serializer&);

  // used by the superfx debugger prior to executing instructions
  debugvirtual void op_step() {};
};

#if defined(DEBUGGER)
  #include "debugger/debugger.hpp"
  extern SFXDebugger superfx;
#else
  extern SuperFX superfx;
#endif
extern SuperFXBus superfxbus;
