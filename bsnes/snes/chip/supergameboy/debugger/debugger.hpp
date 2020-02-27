class SGBDebugger : public SuperGameBoy, public ChipDebugger {
public:

#include "disassembler.hpp"

  bool property(unsigned id, string &name, string &value) {
    return false; // no properties for now
  }
  
  enum Register {
    RegisterPC,
    RegisterAF,
    RegisterBC,
    RegisterDE,
    RegisterHL,
    RegisterSP,
  };
  unsigned getRegister(unsigned id);
  void setRegister(unsigned id, unsigned value);
  
  enum {
    FlagZ,
    FlagN,
    FlagH,
    FlagC,
  };
  bool getFlag(unsigned id);
  void setFlag(unsigned id, bool value);

  void init();

  uint8_t read_gb(uint16_t addr);
  void write_gb(uint16_t addr, uint8_t data);

  enum Usage {
    UsageRead   = 0x80,
    UsageWrite  = 0x40,
    UsageExec   = 0x20,
    UsageOpcode = 0x10,
  };
  
  uint8_t& usage(uint16_t addr);

  uint8 *usage_;
  uint8 *cart_usage; // currently unused

  function<void ()> step_event;
  uint32_t opcode_pc;
  
  SGBDebugger();
  ~SGBDebugger();
  
private:
  function<uint8_t (uint16_t)> sgb_read_gb;
  function<void (uint16_t, uint8_t)> sgb_write_gb;
  
  function<uint32_t (uint16_t)> sgb_addr_with_bank;

  function<uint16_t (char)> sgb_get_reg;
  function<void (char, uint16_t)> sgb_set_reg;
  function<bool (char)> sgb_get_flag;
  function<void (char, bool)> sgb_set_flag;
  
  static void op_call(uint32_t addr);
  static void op_irq(uint32_t addr);
  static void op_ret(uint32_t addr);
  static void op_step(uint32_t pc);
  
  static void op_read(uint32_t addr, uint8_t data);
  static void op_readpc(uint32_t pc, uint8_t data);
  static void op_write(uint32_t addr, uint8_t data);
};
