#include "sgb_processor.hpp"

#define MAX_LINES_PER_DIRECTION 128

// ------------------------------------------------------------------------
SgbDisasmProcessor::SgbDisasmProcessor(SymbolMap *symbols) : symbols(symbols) {
}

// ------------------------------------------------------------------------
string SgbDisasmProcessor::getBreakpointBusName() {
  return "sgb";
}

// ------------------------------------------------------------------------
uint32_t SgbDisasmProcessor::getCurrentAddress() {
  return SNES::supergameboy.opcode_pc;
}

// ------------------------------------------------------------------------
SymbolMap *SgbDisasmProcessor::getSymbols() {
  return symbols;
}

// ------------------------------------------------------------------------
uint32_t SgbDisasmProcessor::findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) {
  unsigned line, i;

  for (line=0; line<linesBelow; line++) {
    for (i=1; i<=4; i++) {
      if ((SNES::supergameboy.usage[(currentAddress + i) & 0xFFFFFF] & SNES::SGBDebugger::UsageOpcode) == 0) {
        continue;
      }

      currentAddress += i;
      break;
    }
  }

  return currentAddress;
}

// ------------------------------------------------------------------------
uint16_t SgbDisasmProcessor::decode(unsigned type, uint16_t address, uint16_t pc) {
  return SNES::supergameboy.decode(type, address, pc);
}

// ------------------------------------------------------------------------
uint16_t SgbDisasmProcessor::decode(SNES::SGBDebugger::Opcode &opcode, uint16_t pc) {
  if (opcode.optype[1]) {
    return decode(opcode.optype[1], opcode.opall(), pc);
  } else {
    return decode(opcode.optype[0], opcode.opall(), pc);
  }
}

// ------------------------------------------------------------------------
void SgbDisasmProcessor::setOpcodePartParams(DisassemblerLine &result, unsigned part, SNES::SGBDebugger::Opcode &opcode, uint16_t pc) {
  string strPart("%", integer(part));
  DisassemblerParam dummy = DisassemblerParam::createValue(0);
  unsigned type = opcode.optype[part - 1];

  switch (type) {
    case GBCPU::Implied:
      result.paramFormat << "";
      break;

    case GBCPU::RegA:
      result.paramFormat << "a";
      result.params.append(dummy);
      break;

    case GBCPU::RegAF:
      result.paramFormat << "af";
      result.params.append(dummy);
      break;

    case GBCPU::RegB:
      result.paramFormat << "b";
      result.params.append(dummy);
      break;

    case GBCPU::RegC:
    case GBCPU::FlagC:
      result.paramFormat << "c";
      result.params.append(dummy);
      break;

    case GBCPU::RegBC:
      result.paramFormat << "bc";
      result.params.append(dummy);
      break;

    case GBCPU::RegD:
      result.paramFormat << "d";
      result.params.append(dummy);
      break;

    case GBCPU::RegE:
      result.paramFormat << "e";
      result.params.append(dummy);
      break;

    case GBCPU::RegDE:
      result.paramFormat << "de";
      result.params.append(dummy);
      break;

    case GBCPU::RegH:
      result.paramFormat << "h";
      result.params.append(dummy);
      break;

    case GBCPU::RegL:
      result.paramFormat << "l";
      result.params.append(dummy);
      break;

    case GBCPU::RegHL:
      result.paramFormat << "hl";
      result.params.append(dummy);
      break;

    case GBCPU::RegSP:
      result.paramFormat << "sp";
      result.params.append(dummy);
      break;

    case GBCPU::FlagZ:
      result.paramFormat << "z";
      result.params.append(dummy);
      break;

    case GBCPU::FlagNC:
      result.paramFormat << "nc";
      result.params.append(dummy);
      break;

    case GBCPU::FlagNZ:
      result.paramFormat << "nz";
      result.params.append(dummy);
      break;

    case GBCPU::RegBCPtr:
      result.paramFormat << "(bc)";
      result.params.append(DisassemblerParam::createAddress(0, decode(type, 0, pc)));
      break;

    case GBCPU::RegCPtr:
      result.paramFormat << "($ff00+c)";
      result.params.append(DisassemblerParam::createAddress(0, decode(type, 0, pc)));
      break;

    case GBCPU::RegDEPtr:
      result.paramFormat << "(de)";
      result.params.append(DisassemblerParam::createAddress(0, decode(type, 0, pc)));
      break;

    case GBCPU::RegHLPtr:
      result.paramFormat << "(hl)";
      result.params.append(DisassemblerParam::createAddress(0, decode(type, 0, pc)));
      break;

    case GBCPU::RegHLPtrInc:
      result.paramFormat << "(hl+)";
      result.params.append(DisassemblerParam::createAddress(0,decode(type, 0, pc)));
      break;

    case GBCPU::RegHLPtrDec:
      result.paramFormat << "(hl-)";
      result.params.append(DisassemblerParam::createAddress(0, decode(type, 0, pc)));
      break;

    case GBCPU::Imm8:
      result.paramFormat << string(strPart, "X2");
      result.params.append(DisassemblerParam::createValue(opcode.op8()));
      break;

    case GBCPU::ImmS8:
      result.paramFormat << string(strPart, "X2");
      result.params.append(DisassemblerParam::createValue(opcode.op8()));
      break;

    case GBCPU::Imm16:
      result.paramFormat << string(strPart, "X4");
      result.params.append(DisassemblerParam::createValue(opcode.op16()));
      break;

    case GBCPU::Addr16:
    case GBCPU::PCRelative:
      result.paramFormat << string(strPart, "X4");
      result.params.append(DisassemblerParam::createAddress(decode(type, opcode.opall(), pc), decode(type, opcode.opall(), pc)));
      break;

    case GBCPU::Addr8Ptr:
    case GBCPU::Addr16Ptr:
      result.paramFormat << string("(", strPart, "X4)");
      result.params.append(DisassemblerParam::createAddress(decode(type, opcode.opall(), pc), decode(type, opcode.opall(), pc)));
      break;

    case GBCPU::RST:
      result.paramFormat << string(strPart, "X2");
      result.params.append(DisassemblerParam::createAddress(opcode.op() & 0x38, opcode.op() & 0x38));
      break;

    case GBCPU::SPRelative:
      if (opcode.op8() & 0x80) {
        result.paramFormat << string("sp", strPart, "D0");
      } else {
        result.paramFormat << string("sp+", strPart, "D0");
      }
      result.params.append(DisassemblerParam::createAddress((int8_t)opcode.op8(), decode(type, opcode.op8(), pc)));
      break;

    default: result.paramFormat = "???"; break;
  }
}

// ------------------------------------------------------------------------
void SgbDisasmProcessor::setOpcodeParams(DisassemblerLine &result, SNES::SGBDebugger::Opcode &opcode, uint16_t address) {
  result.paramFormat = "";
  if (opcode.optype[0] != GBCPU::PrefixCB) {
    setOpcodePartParams(result, 1, opcode, address);
    if (opcode.optype[1] != GBCPU::Implied) {
      result.paramFormat << ", ";
      setOpcodePartParams(result, 2, opcode, address);
    }
  } else {
    result.setOpcode(address, gbPrefixName[opcode.op8() >> 3]);
    SNES::SGBDebugger::Opcode temp = opcode;
    if (opcode.op8() < 0x40) {
      temp.optype[0] = gbPrefixMode[opcode.op8() & 7];
      setOpcodePartParams(result, 1, temp, address);
    } else {
      result.paramFormat = "%1U1, ";
      result.params.append(DisassemblerParam::createValue((opcode.op8() & 0x38) >> 3));
      temp.optype[1] = gbPrefixMode[opcode.op8() & 7];
      setOpcodePartParams(result, 2, temp, address);
    }
  }
}

// ------------------------------------------------------------------------
bool SgbDisasmProcessor::getLine(DisassemblerLine &result, uint32_t &address) {
  SNES::SGBDebugger::Opcode opcode;
  SNES::supergameboy.disassemble_opcode_ex(opcode, address);

  result.setOpcode(address, opcode.opcode);
  setOpcodeParams(result, opcode, address);

  if (opcode.isBra() || opcode.isBraWithContinue()) {
    result.setBra(decode(opcode, address));
  }
  if (opcode.returns()) {
    result.flags |= DisassemblerLine::FLAG_RETURN;
  }

  // Advance to next
  if ((SNES::supergameboy.usage[(address + opcode.size()) & 0xFFFF] & SNES::SGBDebugger::UsageOpcode) != 0) {
    address += opcode.size();
  }

  return true;
}

// ------------------------------------------------------------------------
void SgbDisasmProcessor::analyze(uint32_t address) {
  SNES::SGBDebugger::Opcode op;
  uint32_t maxMethodSize = 0x1000;
  bool force = true;
  uint8_t *usage = SNES::supergameboy.usage;

  while (--maxMethodSize) {
    address = address & 0xFFFF;
    if (usage[address] != 0 && !force) {
      break;
    }

    usage[address] |= SNES::SGBDebugger::UsageOpcode;
    SNES::supergameboy.disassemble_opcode_ex(op, address);

    if (op.isBraWithContinue() && !op.isIndirect()) {
      uint16_t target = decode(op, address);
      if (usage[target] == 0) {
        analyze(target);
      }
    }

    if (op.isBra() && !op.isIndirect()) {
      address = decode(op, address);
      force = false; // we might be branching/jumping into already analyzed code
    } else if (op.returns() || op.isBra()) {
      break;
    } else {
      address += op.size();
    }
  }
}

// ------------------------------------------------------------------------
void SgbDisasmProcessor::findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines) {
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
      if ((SNES::supergameboy.usage[(startAddress - i) & 0xFFFF] & SNES::SGBDebugger::UsageOpcode) == 0) {
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
      if ((SNES::supergameboy.usage[(endAddress + i) & 0xFFFF] & SNES::SGBDebugger::UsageOpcode) == 0) {
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
uint8_t SgbDisasmProcessor::usage(uint32_t address) {
  return SNES::supergameboy.usage[address & 0xFFFF];
}

// ------------------------------------------------------------------------
uint8_t SgbDisasmProcessor::read(uint32_t address) {
  if (!SNES::cartridge.loaded()) {
    return 0;
  }

  SNES::debugger.bus_access = true;
  uint8_t data = SNES::debugger.read(SNES::Debugger::MemorySource::SGBBus, address & 0xFFFF);
  SNES::debugger.bus_access = false;
  return data;
}

// ------------------------------------------------------------------------
void SgbDisasmProcessor::write(uint32_t address, uint8_t data) {
  if (!SNES::cartridge.loaded()) {
    return;
  }

  SNES::debugger.bus_access = true;
  SNES::debugger.write(SNES::Debugger::MemorySource::SGBBus, address & 0xFFFF, data);
  SNES::debugger.bus_access = false;
}

// ------------------------------------------------------------------------
uint32_t SgbDisasmProcessor::getBusSize() {
  return 0x10000;
}
