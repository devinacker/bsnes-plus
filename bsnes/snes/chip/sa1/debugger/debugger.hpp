class SA1Debugger : public SA1, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);
  
  enum {
    RegisterPC,
    RegisterA,
    RegisterX,
    RegisterY,
    RegisterS,
    RegisterD,
    RegisterDB,
    RegisterP,
  };
  unsigned getRegister(unsigned id);
  void     setRegister(unsigned id, unsigned value);
  
  enum {
    FlagE,
    FlagN,
    FlagV,
    FlagM,
    FlagX,
    FlagD,
    FlagI,
    FlagZ,
    FlagC,
  };
  bool     getFlag(unsigned id);
  void     setFlag(unsigned id, bool value);
  
  function<void ()> step_event;

  enum Usage {
    UsageRead   = 0x80,
    UsageWrite  = 0x40,
    UsageExec   = 0x20,
    UsageOpcode = 0x10,
    UsageFlagM  = 0x02,
    UsageFlagX  = 0x01,
  };
  uint8 *usage;
  uint8 **cart_usage;

  uint24 opcode_pc;  //points to the current opcode, used to backtrace on read/write breakpoints

  void interrupt(uint16 vector);
  void op_step();
  uint8_t op_readpc();
  uint8 op_read(uint32 addr);
  void op_write(uint32 addr, uint8 data);

  uint8 disassembler_read(uint32 addr);
  
  SA1Debugger();
  ~SA1Debugger();
};
