class SFXDebugger : public SuperFX, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);

  function<void ()> step_event;

  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
  };
  uint8 *usage;
  uint8 **cart_usage;

  uint24 opcode_pc;  //points to the current opcode, used to backtrace on read/write breakpoints

  bool opcode_edge;  //true right before an opcode execues, used to skip over opcodes

  void op_step();
  
  alwaysinline uint8 peekpipe();
  alwaysinline uint8 pipe();
  uint8 op_read(uint16 addr);
  // void op_write(uint32 addr, uint8 data);

  SFXDebugger();
  ~SFXDebugger();
};
