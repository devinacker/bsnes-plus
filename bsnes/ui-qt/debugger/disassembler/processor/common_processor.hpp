class CommonDisasmProcessor : public DisasmProcessor {
public:
  enum Source { CPU, SMP, SA1, SFX };

  CommonDisasmProcessor(Source);

  void setSource(Source);

  virtual class SymbolMap *getSymbols() {
    return NULL;
  }

  virtual uint32_t getBusSize();
  virtual uint32_t findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow);
  virtual void findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines);
  virtual bool getLine(DisassemblerLine &result, uint32_t &address);
  virtual string getBreakpointBusName();
  virtual uint32_t getCurrentAddress();

  virtual uint8_t usage(uint32_t address);
  virtual uint8_t read(uint32_t address);
  virtual void write(uint32_t address, uint8_t data);

private:
  Source source;
  SNES::Debugger::MemorySource memorySource;

  uint8_t *usagePointer;
  unsigned mask;
};
