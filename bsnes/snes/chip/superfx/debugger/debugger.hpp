class SFXDebugger : public SuperFX, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);
  
  enum Register {
    // 0-15 == R0-R15
    RegisterSFR = 16,
    // TODO: some other registers here (ROMBR, etc)
  };
  unsigned getRegister(unsigned id);
  void     setRegister(unsigned id, unsigned value);

  enum {
    FlagI,
    FlagB,
    FlagIH,
    FlagIL,
    FlagA2,
    FlagA1,
    FlagR,
    FlagG,
    FlagV,
    FlagN,
    FlagC,
    FlagZ,
  };
  bool     getFlag(unsigned id);
  void     setFlag(unsigned id, bool value);
  
  function<void ()> step_event;

  enum Usage {
    UsageRead   = 0x80,
    UsageWrite  = 0x40,
    UsageExec   = 0x20,
    UsageOpcode = 0x10,
  };
  uint8 *usage;
  uint8 **cart_usage;

  uint24 opcode_pc;  //points to the current opcode, used to backtrace on read/write breakpoints
  bool pc_valid;

  void reset();

  void op_step();
  
  // mark pipelined instruction bytes as executed and update last pipeline read address
  uint8 op_read(uint16 addr);
  
  // mark (and break on) buffered i/o
  uint8 rombuffer_read();

  uint8 rambuffer_read(uint16 addr);
  void rambuffer_write(uint16 addr, uint8 data);

  SFXDebugger();
  ~SFXDebugger();
};
