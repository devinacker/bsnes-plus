class SgbDisasmProcessor : public DisasmProcessor {
public:
  SgbDisasmProcessor(SymbolMap*);

  virtual class SymbolMap *getSymbols();

  virtual uint32_t getBusSize();
  virtual uint32_t findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow);
  virtual void findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines);
  virtual bool getLine(DisassemblerLine &result, uint32_t &address);
  virtual void analyze(uint32_t address);
  virtual string getBreakpointBusName();
  virtual uint32_t getCurrentAddress();

  virtual uint8_t usage(uint32_t address);
  virtual uint8_t read(uint32_t address);
  virtual void write(uint32_t address, uint8_t data);

private:
  SymbolMap *symbols;
  uint8_t *usagePointer;

  uint16_t decode(unsigned type, uint16_t address, uint16_t pc);
  uint16_t decode(SNES::SGBDebugger::Opcode &opcode, uint16_t pc);
  void setOpcodePartParams(DisassemblerLine &result, unsigned part, SNES::SGBDebugger::Opcode &opcode, uint16_t address);
  void setOpcodeParams(DisassemblerLine &result, SNES::SGBDebugger::Opcode &opcode, uint16_t address);

};
