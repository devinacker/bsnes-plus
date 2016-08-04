class SMPDebugger : public SMP, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);

  enum Register {
    RegisterPC,
    RegisterA,
    RegisterX,
    RegisterY,
    RegisterS,
    RegisterYA,
    RegisterP,
  };
  unsigned getRegister(unsigned id);
  void     setRegister(unsigned id, unsigned value);

  enum {
    FlagN,
    FlagV,
    FlagP,
    FlagB,
    FlagH,
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
  };
  uint8 *usage;
  uint16 opcode_pc;

  void op_step();
  uint8_t op_readpc();
  uint8 op_read(uint16 addr);
  void op_write(uint16 addr, uint8 data);

  SMPDebugger();
  ~SMPDebugger();
};
