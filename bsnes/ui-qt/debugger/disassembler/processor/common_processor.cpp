#include "common_processor.hpp"

#define MAX_LINES_PER_DIRECTION 128

// ------------------------------------------------------------------------
CommonDisasmProcessor::CommonDisasmProcessor(CommonDisasmProcessor::Source source) {
  setSource(source);
}

// ------------------------------------------------------------------------
void CommonDisasmProcessor::setSource(Source source) {
  this->source = source;

  switch (source) {
  case CPU:
    usage = SNES::cpu.usage;
    mask = (1 << 24) - 1;
    break;

  case SMP:
    usage = SNES::smp.usage;
    mask = (1 << 16) - 1;
    break;

  case SA1:
    usage = SNES::sa1.usage;
    mask = (1 << 24) - 1;
    break;

  case SFX:
    usage = SNES::superfx.usage;
    mask = (1 << 23) - 1;
    break;
  }
}

// ------------------------------------------------------------------------
uint32_t CommonDisasmProcessor::findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) {
  unsigned line, i;

  for (line=0; line<linesBelow; line++) {
    for (i=1; i<=4; i++) {
      if ((usage[(currentAddress + i) & mask] & 0x10) == 0) {
        continue;
      }

      currentAddress += i;
      break;
    }
  }

  return currentAddress;
}

// ------------------------------------------------------------------------
bool CommonDisasmProcessor::getLine(DisassemblerLine &result, uint32_t &address) {
  char t[256];
  char *op;

  switch (source) {
  case CPU:
    SNES::cpu.disassemble_opcode(t, address);
    t[20] = 0;
    op = &t[7];
    break;

  case SMP:
    SNES::smp.disassemble_opcode(t, address);
    t[23] = 0;
    op = &t[7];
    break;

  case SA1:
    SNES::sa1.disassemble_opcode(t, address);
    t[20] = 0;
    op = &t[7];
    break;

  case SFX:
    SNES::superfx.disassemble_opcode(t, address);
    t[25] = 0;
    op = &t[7];
    break;
  }

  string text = rtrim(op);

  result.setOpcode(address, text);

  for (uint32_t i=1; i<=4; i++) {
    if ((usage[(address + i) & mask] & 0x10) == 0) {
      continue;
    }

    address += i;
    break;
  }

  return true;
}

// ------------------------------------------------------------------------
void CommonDisasmProcessor::findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines) {
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
      if ((usage[(startAddress - i) & mask] & 0x10) == 0) {
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
      if ((usage[(endAddress + i) & mask] & 0x10) == 0) {
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
uint32_t CommonDisasmProcessor::getBusSize() {
  return 0x1000000;
}
