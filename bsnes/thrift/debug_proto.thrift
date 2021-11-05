enum BsnesRegister {
  pc,
  a,
  x,
  y,
  s,
  d,
  db,
  p,
  mflag,
  xflag,
  eflag,
}

struct BsnesRegisters {
  1:i32 pc,
  2:i32 a,
  3:i32 x,
  4:i32 y,
  5:i32 s,
  6:i32 d,
  7:i16 db,
  8:i16 p,
  9:i8 mflag,
  10:i8 xflag,
  11:i8 eflag,
}

enum BpType {
  BP_PC = 1,
  BP_READ = 2,
  BP_WRITE = 4,
}

enum DbgMemorySource {
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
}

enum DbgBptSource {
  CPUBus,
  APURAM,
  DSP,
  VRAM,
  OAM,
  CGRAM,
  SA1Bus,
  SFXBus,
  SGBBus,
}

struct DbgBreakpoint {
  1:BpType type,
  2:i32 bstart,
  3:i32 bend,
  4:bool enabled,
  5:DbgBptSource src,
}

service BsnesDebugger {
  i32 get_cpu_reg(1:BsnesRegister reg),
  BsnesRegisters get_cpu_regs(),
  void set_cpu_reg(1:BsnesRegister reg, 2:i32 value),

  binary read_memory(1:DbgMemorySource src, 2:i32 address, 3:i32 size),
  void write_memory(1:DbgMemorySource src, 2:i32 address, 3:binary data),

  void add_breakpoint(1:DbgBreakpoint bpt),
  void del_breakpoint(1:DbgBreakpoint bpt),

  void pause(),
  void resume(),
  void start_emulation(),
  void exit_emulation(),

  void step_into(),
  void step_over(),
}

service IdaClient {
  oneway void start_event(),
  oneway void add_visited(1:set<i32> changed, 2:bool is_step),
  oneway void pause_event(1:i32 address),
  oneway void stop_event(),
}
