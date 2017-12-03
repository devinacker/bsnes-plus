#include "../line.hpp"

class DisasmProcessor {
public:
  virtual uint32_t getBusSize() = 0;
  virtual uint32_t getCurrentAddress() = 0;

  virtual class SymbolMap *getSymbols() = 0;

  virtual uint32_t findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) = 0;
  virtual void findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines) = 0;
  virtual bool getLine(DisassemblerLine &result, uint32_t &address) = 0;

  virtual uint8_t usage(uint32_t address) = 0;
  virtual uint8_t read(uint32_t address) = 0;
  virtual void write(uint32_t address, uint8_t data) = 0;

  virtual string getBreakpointBusName() = 0;
};
