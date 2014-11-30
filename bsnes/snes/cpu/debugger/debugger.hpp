class CPUDebugger : public CPU, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);

  function<void ()> step_event;

  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
    UsageFlagM = 0x02,
    UsageFlagX = 0x01,
  };
  uint8 *usage;
  uint8 *cart_usage;
#if defined(ALT_CPU_HPP)
  uint32 opcode_pc;
#else
  uint24 opcode_pc;  //points to the current opcode, used to backtrace on read/write breakpoints
#endif
  bool opcode_edge;  //true right before an opcode execues, used to skip over opcodes

  void op_step();
  virtual uint8_t op_readpc();
  uint8 op_read(uint32 addr);
  void op_write(uint32 addr, uint8 data);

  CPUDebugger();
  ~CPUDebugger();
};
