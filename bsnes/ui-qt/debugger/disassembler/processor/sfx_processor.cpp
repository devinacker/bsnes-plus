#include "sfx_processor.hpp"

#define MAX_LINES_PER_DIRECTION 128

// ------------------------------------------------------------------------
SfxDisasmProcessor::SfxDisasmProcessor(SymbolMap *symbols) : symbols(symbols) {
}

// ------------------------------------------------------------------------
string SfxDisasmProcessor::getBreakpointBusName() {
  return "sfx";
}

// ------------------------------------------------------------------------
uint32_t SfxDisasmProcessor::getCurrentAddress() {
  return SNES::superfx.opcode_pc;
}

// ------------------------------------------------------------------------
SymbolMap *SfxDisasmProcessor::getSymbols() {
  return symbols;
}

// ------------------------------------------------------------------------
uint32_t SfxDisasmProcessor::findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) {
  unsigned line, i;

  for (line=0; line<linesBelow; line++) {
    for (i=1; i<=4; i++) {
      if ((SNES::superfx.usage[(currentAddress + i) & 0x7FFFFF] & SNES::SFXDebugger::UsageOpcode) == 0) {
        continue;
      }

      currentAddress += i;
      break;
    }
  }

  return currentAddress;
}

// ------------------------------------------------------------------------
uint32_t SfxDisasmProcessor::decode(uint32_t type, uint32_t address, uint32_t pc) {
  return SNES::superfx.decode(type, address, pc);
}

// ------------------------------------------------------------------------
void SfxDisasmProcessor::setOpcodeParams(DisassemblerLine &result, SNES::SuperFX::Opcode &opcode, uint32_t pc) {
  uint32_t addr = decode(opcode.optype, opcode.opall(), pc);

  switch (opcode.optype) {
    case SNES::SuperFX::Implied:
    case SNES::SuperFX::ImpliedPlot:
    case SNES::SuperFX::ImpliedLongMul:
    case SNES::SuperFX::ImpliedFixedMul:
    case SNES::SuperFX::ImpliedMerge:
    case SNES::SuperFX::ImpliedLoop:
      result.paramFormat = "";
      break;

    case SNES::SuperFX::ImpliedROM:
    case SNES::SuperFX::ImpliedRAM:
      result.paramFormat = "";
      result.params.append(DisassemblerParam::createAddress(addr, addr));
      break;

    case SNES::SuperFX::Relative:
    case SNES::SuperFX::LinkRelative:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress((uint16_t)addr, addr));
      break;
    
    case SNES::SuperFX::Register:
      result.paramFormat = string("r", opcode.op4());
      break;
    
    case SNES::SuperFX::RegisterJump:
    case SNES::SuperFX::RegisterLJump:
      result.paramFormat = string("r", opcode.op4());
      result.params.append(DisassemblerParam::createAddress(addr, addr));
      break;
    
    case SNES::SuperFX::IRegister:
      result.paramFormat = string("(r", opcode.op4(), ")");
      result.params.append(DisassemblerParam::createAddress(addr, addr));
      break;
    
    case SNES::SuperFX::RegImmediate:
      result.paramFormat = string("r", opcode.op4(), ", #%1X2");
      result.params.append(DisassemblerParam::createValue(opcode.op8()));
      break;

    case SNES::SuperFX::RegImmediateLong:
      result.paramFormat = string("r", opcode.op4(), ", #%1X4");
      result.params.append(DisassemblerParam::createAddress(opcode.op16(), addr));
      break;

    case SNES::SuperFX::RegDirect:
      result.paramFormat = string("r", opcode.op4(), ", %1X3");
      result.params.append(DisassemblerParam::createAddress((uint16_t)addr, addr));
      break;

    case SNES::SuperFX::RegAbsolute:
      result.paramFormat = string("r", opcode.op4(), ", %1X4");
      result.params.append(DisassemblerParam::createAddress(opcode.op16(), addr));
      break;

    case SNES::SuperFX::ImmediateShort:
      result.paramFormat = "#%1X2";
      result.params.append(DisassemblerParam::createValue(opcode.op4()));
      break;

    default: result.paramFormat = "???"; break;
  }
}

// ------------------------------------------------------------------------
bool SfxDisasmProcessor::getLine(DisassemblerLine &result, uint32_t &address) {
  SNES::SuperFX::Opcode opcode;

  uint8_t u = SNES::superfx.usage[address & 0x7FFFFF];
  bool alt1 = u & SNES::SFXDebugger::UsageFlagA1;
  bool alt2 = u & SNES::SFXDebugger::UsageFlagA2;

  SNES::superfx.disassemble_opcode_ex(opcode, address, alt1, alt2);
  
  result.setOpcode(address, opcode.opcode);
  setOpcodeParams(result, opcode, address);

  if (opcode.isBra() || opcode.isBraWithContinue()) {
    result.setBra(decode(opcode.optype, opcode.opall(), address));
  }
  if (opcode.returns()) {
    result.flags |= DisassemblerLine::FLAG_RETURN;
  }

  // Advance to next
  if ((SNES::superfx.usage[(address + opcode.size()) & 0x7FFFFF] & SNES::SFXDebugger::UsageOpcode) != 0) {
    address += opcode.size();
  }

  return true;
}

// ------------------------------------------------------------------------
void SfxDisasmProcessor::findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines) {
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
      if ((SNES::superfx.usage[(startAddress - i) & 0x7FFFFF] & SNES::SFXDebugger::UsageOpcode) == 0) {
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
      if ((SNES::superfx.usage[(endAddress + i) & 0x7FFFFF] & SNES::SFXDebugger::UsageOpcode) == 0) {
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
uint8_t SfxDisasmProcessor::usage(uint32_t address) {
  return SNES::superfx.usage[address & 0x7FFFFF];
}

// ------------------------------------------------------------------------
uint8_t SfxDisasmProcessor::read(uint32_t address) {
  if (!SNES::cartridge.loaded()) {
    return 0;
  }

  SNES::debugger.bus_access = true;
  uint8_t data = SNES::debugger.read(SNES::Debugger::MemorySource::SFXBus, address & 0x7FFFFF);
  SNES::debugger.bus_access = false;
  return data;
}

// ------------------------------------------------------------------------
void SfxDisasmProcessor::write(uint32_t address, uint8_t data) {
  if (!SNES::cartridge.loaded()) {
    return;
  }

  SNES::debugger.bus_access = true;
  SNES::debugger.write(SNES::Debugger::MemorySource::SFXBus, address & 0x7FFFFF, data);
  SNES::debugger.bus_access = false;
}

// ------------------------------------------------------------------------
uint32_t SfxDisasmProcessor::getBusSize() {
  return 0x800000;
}
