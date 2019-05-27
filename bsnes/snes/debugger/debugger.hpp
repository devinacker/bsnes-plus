class Debugger {
public:
  enum class BreakEvent : unsigned {
    None,
    BreakpointHit,
    CPUStep,
    SMPStep,
    SA1Step,
    SFXStep,
  } break_event;

  enum { Breakpoints = 8,
         SoftBreakCPU = Breakpoints,
         SoftBreakSA1, };
  struct Breakpoint {
    bool enabled;
    unsigned addr;
    unsigned addr_end; //0 = unused
    signed data;  //-1 = unused
    
    enum class Mode : unsigned { Exec = 1, Read = 2, Write = 4 };
    unsigned mode;
    
    enum class Source : unsigned { CPUBus, APURAM, VRAM, OAM, CGRAM, SA1Bus, SFXBus } source;
    unsigned counter;  //number of times breakpoint has been hit since being set
  } breakpoint[Breakpoints];
  unsigned breakpoint_hit;
  void breakpoint_test(Breakpoint::Source source, Breakpoint::Mode mode, unsigned addr, uint8 data);

  bool step_cpu;
  bool step_smp;
  bool step_sa1;
  bool step_sfx;
  bool bus_access;
  bool break_on_wdm;
  bool break_on_brk;

  enum class StepType : unsigned {
    None, StepToNMI, StepToIRQ, StepToVBlank, StepToHBlank, StepInto, StepOver, StepOut
  } step_type;
  int call_count;
  bool step_over_new;

  enum class MemorySource : unsigned { CPUBus, APUBus, APURAM, VRAM, OAM, CGRAM, CartROM, CartRAM, SA1Bus, SFXBus };
  uint8 read(MemorySource, unsigned addr);
  void write(MemorySource, unsigned addr, uint8 data);

  Debugger();
};

extern Debugger debugger;
