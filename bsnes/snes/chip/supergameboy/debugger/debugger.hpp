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

  enum Usage {
    UsageRead   = 0x80,
    UsageWrite  = 0x40,
    UsageExec   = 0x20,
    UsageOpcode = 0x10,
  };
  uint8 *usage; // currently unused
  uint8 *cart_usage;
  
  SGBDebugger();
  ~SGBDebugger();
  
private:
  function<uint8 (uint16)> sgb_read_gb;
  function<void (uint16, uint8)> sgb_write_gb;

};
