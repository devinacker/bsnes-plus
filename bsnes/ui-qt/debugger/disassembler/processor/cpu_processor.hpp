class CpuDisasmProcessor : public DisasmProcessor {
public:
  enum Source { CPU, SA1 };

  CpuDisasmProcessor(Source, SymbolMap*);

  void setSource(Source);

  virtual class SymbolMap *getSymbols();

  virtual uint32_t getBusSize();
  virtual uint32_t findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow);
  virtual void findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines);
  virtual bool getLine(DisassemblerLine &result, uint32_t &address);

private:
  Source source;

  SymbolMap *symbols;
  uint8_t *usage;

  uint32_t decode(uint32_t type, uint32_t address, uint32_t pc);
  void setOpcodeParams(DisassemblerLine &result, SNES::CPU::Opcode &opcode, uint32_t address);

};
