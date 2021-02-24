#include "common_processor.hpp"

#define MAX_LINES_PER_DIRECTION 128

// ------------------------------------------------------------------------
CommonDisasmProcessor::CommonDisasmProcessor(CommonDisasmProcessor::Source source) {
  setSource(source);
}

// ------------------------------------------------------------------------
string CommonDisasmProcessor::getBreakpointBusName() {
  switch (source) {
    case CPU: return "cpu";
    case SMP: return "smp";
    case SFX: return "sfx";
    case SA1: return "sa1";
    case SGB: return "sgb";
  }

  return "";
}

// ------------------------------------------------------------------------
uint32_t CommonDisasmProcessor::getCurrentAddress() {
  switch (source) {
    case CPU: return SNES::cpu.opcode_pc;
    case SMP: return SNES::smp.opcode_pc;
    case SFX: return SNES::superfx.opcode_pc;
    case SA1: return SNES::sa1.opcode_pc;
    case SGB: return SNES::supergameboy.opcode_pc;
  }
  
  return 0;
}

// ------------------------------------------------------------------------
void CommonDisasmProcessor::setSource(Source source) {
  this->source = source;

  switch (source) {
  case CPU:
    usagePointer = SNES::cpu.usage;
    memorySource = SNES::Debugger::MemorySource::CPUBus;
    mask = (1 << 24) - 1;
    break;

  case SMP:
    usagePointer = SNES::smp.usage;
    memorySource = SNES::Debugger::MemorySource::APUBus;
    mask = (1 << 16) - 1;
    break;

  case SA1:
    usagePointer = SNES::sa1.usage;
    memorySource = SNES::Debugger::MemorySource::SA1Bus;
    mask = (1 << 24) - 1;
    break;

  case SFX:
    usagePointer = SNES::superfx.usage;
    memorySource = SNES::Debugger::MemorySource::SFXBus;
    mask = (1 << 23) - 1;
    break;

  case SGB:
    usagePointer = SNES::supergameboy.usage_;
    memorySource = SNES::Debugger::MemorySource::SGBBus;
    mask = (1 << 16) - 1;
    break;
  }
}

// ------------------------------------------------------------------------
uint32_t CommonDisasmProcessor::findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) {
  unsigned line, i;

  for (line=0; line<linesBelow; line++) {
    for (i=1; i<=4; i++) {
      if ((usagePointer[(currentAddress + i) & mask] & SNES::CPUDebugger::UsageOpcode) == 0) {
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

  case SGB:
    SNES::supergameboy.disassemble_opcode(t, address);
    t[25] = 0;
    op = &t[7];
    break;
  }

  string text = rtrim(op);

  result.setOpcode(address, text);

  for (uint32_t i=1; i<=4; i++) {
    if ((usagePointer[(address + i) & mask] & SNES::CPUDebugger::UsageOpcode) == 0) {
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
      if ((usagePointer[(startAddress - i) & mask] & SNES::CPUDebugger::UsageOpcode) == 0) {
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
      if ((usagePointer[(endAddress + i) & mask] & SNES::CPUDebugger::UsageOpcode) == 0) {
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
uint8_t CommonDisasmProcessor::usage(uint32_t address) {
  return usagePointer[address & mask];
}

// ------------------------------------------------------------------------
uint8_t CommonDisasmProcessor::read(uint32_t address) {
  if (SNES::cartridge.loaded()) {
    SNES::debugger.bus_access = true;
    uint8_t data = SNES::debugger.read(memorySource, address & mask);
    SNES::debugger.bus_access = false;
    return data;
  }

  return 0;
}

// ------------------------------------------------------------------------
void CommonDisasmProcessor::write(uint32_t address, uint8_t data) {
  if (SNES::cartridge.loaded()) {
    SNES::debugger.bus_access = true;
    SNES::debugger.write(memorySource, address & mask, data);
    SNES::debugger.bus_access = false;
  }
}

// ------------------------------------------------------------------------
uint32_t CommonDisasmProcessor::getBusSize() {
  return 0x1000000;
}
