#include "smp_processor.hpp"

// ------------------------------------------------------------------------
SmpDisasmProcessor::SmpDisasmProcessor(SymbolMap* map) : symbols(map) {
}

// ------------------------------------------------------------------------
string SmpDisasmProcessor::getBreakpointBusName() {
  return "smp";
}

// ------------------------------------------------------------------------
uint32_t SmpDisasmProcessor::getCurrentAddress() {
  return SNES::smp.opcode_pc;
}

// ------------------------------------------------------------------------
uint32_t SmpDisasmProcessor::findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) {
  unsigned line, i;

  for (line=0; line<linesBelow; line++) {
    for (i=1; i<=4; i++) {
      if ((SNES::smp.usage[(currentAddress + i) & 0xFFFF] & 0x10) == 0) {
        continue;
      }

      currentAddress += i;
      break;
    }
  }

  return currentAddress;
}

// ------------------------------------------------------------------------
bool SmpDisasmProcessor::getLine(DisassemblerLine &result, uint32_t &address) {
  char t[256];
  char *op;

  SNES::smp.disassemble_opcode(t, address);
  t[23] = 0;
  op = &t[7];

  string text = rtrim(op);

  result.setOpcode(address, text);

  for (uint32_t i=1; i<=4; i++) {
    if ((SNES::smp.usage[(address + i) & 0xFFFF] & 0x10) == 0) {
      continue;
    }

    address += i;
    break;
  }

  return true;
}

// ------------------------------------------------------------------------
void SmpDisasmProcessor::findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines) {
  bool result;
  unsigned line, i;

  startAddress = currentAddress;
  endAddress = currentAddress;
  currentAddressLine = 0;
  numLines = 1;

  // Search upwards
  for (line=0; line<MAX_LINES_PER_DIRECTION; line++) {
    result = false;

    for (i=1; i<=4; i++) {
      if ((SNES::smp.usage[(startAddress - i) & 0xFFFF] & 0x10) == 0) {
        continue;
      }

      startAddress -= i;
      numLines++;
      currentAddressLine++;
      result = true;
      break;
    }

    if (!result) {
      break;
    }
  }

  // Search downwards
  for (line=0; line<MAX_LINES_PER_DIRECTION; line++) {
    result = false;

    for (i=1; i<=4; i++) {
      if ((SNES::smp.usage[(endAddress + i) & 0xFFFF] & 0x10) == 0) {
        continue;
      }

      endAddress += i;
      numLines++;
      result = true;
      break;
    }

    if (!result) {
      break;
    }
  }
}

// ------------------------------------------------------------------------
uint8_t SmpDisasmProcessor::usage(uint32_t address) {
  return SNES::smp.usage[address & 0xFFFF];
}

// ------------------------------------------------------------------------
uint8_t SmpDisasmProcessor::read(uint32_t address) {
  if (SNES::cartridge.loaded()) {
    SNES::debugger.bus_access = true;
    uint8_t data = SNES::debugger.read(SNES::Debugger::MemorySource::APUBus, address & 0xFFFF);
    SNES::debugger.bus_access = false;
    return data;
  }

  return 0;
}

// ------------------------------------------------------------------------
void SmpDisasmProcessor::write(uint32_t address, uint8_t data) {
  if (SNES::cartridge.loaded()) {
    SNES::debugger.bus_access = true;
    SNES::debugger.write(SNES::Debugger::MemorySource::APUBus, address & 0xFFFF, data);
    SNES::debugger.bus_access = false;
  }
}

// ------------------------------------------------------------------------
uint32_t SmpDisasmProcessor::getBusSize() {
  return 0x10000;
}
