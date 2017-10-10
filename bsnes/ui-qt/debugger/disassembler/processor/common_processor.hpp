class CommonDisasmProcessor : public DisasmProcessor {
public:
  enum Source { CPU, SMP, SA1, SFX };

  CommonDisasmProcessor(Source);

  void setSource(Source);

  virtual uint32_t getBusSize();
  virtual uint32_t findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow);
  virtual void findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines);
  virtual bool getLine(DisassemblerLine &result, uint32_t &address);

private:
  Source source;

  uint8_t *usage;
  unsigned mask;
};
