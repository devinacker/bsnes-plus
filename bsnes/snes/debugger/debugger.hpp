class Debugger {
public:
  enum class BreakEvent : unsigned {
    None,
    BreakpointHit,
    CPUStep,
    SMPStep,
    SA1Step,
    SFXStep,
    SGBStep,
  } break_event;

  enum { SoftBreakCPU = -1,
         SoftBreakSA1 = -2, };
  
  struct Breakpoint {
    unsigned addr = 0;
    unsigned addr_end = 0; //0 = unused
    signed data = -1;  //-1 = unused
    enum class Compare : unsigned { Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual } compare = Compare::Equal;
    
    // compare breakpoint against read/written data using specified value and comparator
    alwaysinline bool operator==(const uint8& data) const;
    alwaysinline bool operator!=(const uint8& data) const;
    
    enum class Mode : unsigned { Exec = 1, Read = 2, Write = 4 };
    unsigned mode = 0;
    
    enum class Source : unsigned {
      CPUBus,
      APURAM,
      DSP,
      VRAM,
      OAM,
      CGRAM,
      SA1Bus,
      SFXBus,
      SGBBus,
    } source = Source::CPUBus;
    unsigned counter = 0;  //number of times breakpoint has been hit since being set
  };
  linear_vector<Breakpoint> breakpoint;
  unsigned breakpoint_hit;
  void breakpoint_test(Breakpoint::Source source, Breakpoint::Mode mode, unsigned addr, uint8 data);

  bool step_cpu;
  bool step_smp;
  bool step_sa1;
  bool step_sfx;
  bool step_sgb;
  bool bus_access;
  bool break_on_wdm;
  bool break_on_brk;
  bool log_without_break;

  enum class StepType : unsigned {
    None, StepToNMI, StepToIRQ, StepToVBlank, StepToHBlank, StepInto, StepOver, StepOut
  } step_type;
  int call_count;
  bool step_over_new;

  enum class MemorySource : unsigned { 
    CPUBus,
    APUBus,
    APURAM,
    DSP,
    VRAM,
    OAM,
    CGRAM,
    CartROM,
    CartRAM,
    SA1Bus,
    SFXBus,
    SGBBus,
    SGBROM,
    SGBRAM,
  };
  uint8 read(MemorySource, unsigned addr);
  void write(MemorySource, unsigned addr, uint8 data);

  Debugger();
};

extern Debugger debugger;
