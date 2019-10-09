/*
 * Building with the "performance" profile will include this from snes/alt/cpu/cpu.hpp instead of
 * the usual snes/cpu/cpu.hpp. When this is the case, ALT_CPU_HPP is defined.
 * Be sure to test builds with multiple profiles and account for differences in the two implementations.
 */
class CPUDebugger : public CPU, public ChipDebugger {
public:
  bool property(unsigned id, string &name, string &value);
  
  enum Register {
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
    UsageFlagE  = 0x04,
    UsageFlagM  = 0x02,
    UsageFlagX  = 0x01,
  };
  uint8 *usage;
  uint8 *cart_usage;
#if defined(ALT_CPU_HPP)
  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);
  
  void op_irq(uint16 vector);
#else
  uint8 mmio_r2180();
  void mmio_w2180(uint8 data);
  
  void op_irq();
#endif

  uint24 opcode_pc;  //points to the current opcode, used to backtrace on read/write breakpoints
  
  void op_step();
  uint8_t op_readpc();
  uint8 op_read(uint32 addr);
  uint8 dma_read(uint32 abus);
  void op_write(uint32 addr, uint8 data);

  uint8 disassembler_read(uint32 addr);
  uint8 hvbjoy();

  CPUDebugger();
  ~CPUDebugger();
};
