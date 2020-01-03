class SGBDebugger : public SuperGameBoy, public ChipDebugger {
public:
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
  unsigned getRegister(unsigned id) {
    return 0; // TODO
  }
  void setRegister(unsigned id, unsigned value) {} // TODO
  
  enum {
    FlagZ,
    FlagN,
    FlagH,
    FlagC,
  };
  bool getFlag(unsigned id) {
    return false; // TODO
  }
  void setFlag(unsigned id, bool value) {} // TODO

  void init();

  uint8_t read_gb(uint16_t addr);
  void write_gb(uint16_t addr, uint8_t data);

  void disassemble_opcode(char *output, uint16_t addr);

  enum Usage {
    UsageRead   = 0x80,
    UsageWrite  = 0x40,
    UsageExec   = 0x20,
    UsageOpcode = 0x10,
  };
  uint8 *usage; // currently unused
  uint8 *cart_usage;
  
  function<void ()> step_event;
  uint16_t opcode_pc;
  
  SGBDebugger();
  ~SGBDebugger();
  
private:
  function<uint8 (uint16_t)> sgb_read_gb;
  function<void (uint16_t, uint8_t)> sgb_write_gb;
  
  static void op_step(uint16_t pc);
  static void op_read(uint16_t addr, uint8_t data);
  static void op_readpc(uint16_t pc, uint8_t data);
  static void op_write(uint16_t addr, uint8_t data);
};
