#include "cpu_processor.hpp"

#define MAX_LINES_PER_DIRECTION 128

// ------------------------------------------------------------------------
CpuDisasmProcessor::CpuDisasmProcessor(CpuDisasmProcessor::Source source, SymbolMap *symbols) : symbols(symbols) {
  setSource(source);
}

// ------------------------------------------------------------------------
string CpuDisasmProcessor::getBreakpointBusName() {
  switch (source) {
    case CPU: return "cpu";
    case SA1: return "sa1";
  }

  return "";
}

// ------------------------------------------------------------------------
uint32_t CpuDisasmProcessor::getCurrentAddress() {
  switch (source) {
    case CPU: return SNES::cpu.opcode_pc;
    case SA1: return SNES::sa1.opcode_pc;
  }
  
  return 0;
}

// ------------------------------------------------------------------------
SymbolMap *CpuDisasmProcessor::getSymbols() {
  return symbols;
}

// ------------------------------------------------------------------------
void CpuDisasmProcessor::setSource(Source source) {
  this->source = source;

  switch (source) {
  case CPU: usagePointer = SNES::cpu.usage; break;
  case SA1: usagePointer = SNES::sa1.usage; break;
  }
}

// ------------------------------------------------------------------------
uint32_t CpuDisasmProcessor::findStartLineAddress(uint32_t currentAddress, uint32_t linesBelow) {
  unsigned line, i;

  for (line=0; line<linesBelow; line++) {
    for (i=1; i<=4; i++) {
      if ((usagePointer[(currentAddress + i) & 0xFFFFFF] & SNES::CPUDebugger::UsageOpcode) == 0) {
        continue;
      }

      currentAddress += i;
      break;
    }
  }

  return currentAddress;
}

// ------------------------------------------------------------------------
uint32_t CpuDisasmProcessor::decode(uint32_t type, uint32_t address, uint32_t pc) {
  return SNES::cpu.decode(type, address, pc);
}

// ------------------------------------------------------------------------
void CpuDisasmProcessor::setOpcodeParams(DisassemblerLine &result, SNES::CPU::Opcode &opcode, uint32_t pc) {
  switch (opcode.optype) {
    case SNESCPU::Direct:
      result.paramFormat = "%1X2";
      result.params.append(DisassemblerParam::createAddress(opcode.op8(),decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::DirectX:
      result.paramFormat = "%1X2,x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::DirectY:
      result.paramFormat = "%1X2,y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::IDirect:
      result.paramFormat = "(%1X2)";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::IDirectX:
      result.paramFormat = "(%1X2,x)";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::IDirectY:
      result.paramFormat = "(%1X2),y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::ILDirect:
      result.paramFormat = "[%1X2]";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::ILDirectY:
      result.paramFormat = "[%1X2],y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             decode(SNESCPU::Direct, opcode.op8(), pc), decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::Address:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress(opcode.op16(), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::AddressX:
      result.paramFormat = "%1X4,x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             decode(SNESCPU::Address, opcode.op16(), pc), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::AddressY:
      result.paramFormat = "%1X4,y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             decode(SNESCPU::Address, opcode.op16(), pc), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::IAddressX:
      result.paramFormat = "(%1X4,x)";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             decode(SNESCPU::Address, opcode.op16(), pc), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::ILAddress:
      result.paramFormat = "[%1X4]";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             decode(SNESCPU::Address, opcode.op16(), pc), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::Long:
      result.paramFormat = "%1X6";
      result.params.append(DisassemblerParam::createAddress(opcode.op24(), decode(opcode.optype, opcode.op24(), pc)));
      break;

    case SNESCPU::LongX:
      result.paramFormat = "%1X6,x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op24(),
                             decode(SNESCPU::Long, opcode.op24(), pc), decode(opcode.optype, opcode.op24(), pc)));
      break;

    case SNESCPU::Stack:
      result.paramFormat = "%1X2,s";
      result.params.append(DisassemblerParam::createAddress(opcode.op8(),decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::IStackY:
      result.paramFormat = "(%1X2,s),y";
      result.params.append(DisassemblerParam::createAddress(opcode.op8(),decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::PAddress:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress(opcode.op16(), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::PIAddress:
      result.paramFormat = "(%1X4)";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             decode(SNESCPU::Address, opcode.op16(), pc), decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::RelativeShort:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress((uint16_t)decode(opcode.optype, opcode.op8(), pc), 
                             decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESCPU::RelativeLong:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress((uint16_t)decode(opcode.optype, opcode.op16(), pc), 
                             decode(opcode.optype, opcode.op16(), pc)));
      break;

    case SNESCPU::Implied:
      result.paramFormat = "";
      break;

    case SNESCPU::BlockMove:
      result.paramFormat = "%1X2, %2X2";
      result.params.append(DisassemblerParam::createValue(opcode.op8(1)));
      result.params.append(DisassemblerParam::createValue(opcode.op8(0)));
      break;

    case SNESCPU::Constant:
    case SNESCPU::AccumConstant:
    case SNESCPU::IndexConstant:
      if (opcode.paramsize == 1) {
        result.paramFormat = "#%1X2";
        result.params.append(DisassemblerParam::createValue(opcode.op8()));
      } else {
        result.paramFormat = "#%1X4";
        result.params.append(DisassemblerParam::createValue(opcode.op16()));
      }
      break;


    default: result.paramFormat = "???"; break;
  }
}

// ------------------------------------------------------------------------
bool CpuDisasmProcessor::getLine(DisassemblerLine &result, uint32_t &address) {
  SNES::CPU::Opcode opcode;
  uint8_t u = usagePointer[address & 0xFFFFFF];
  bool e, m, x;

  e = u & SNES::CPUDebugger::UsageFlagE;
  m = u & SNES::CPUDebugger::UsageFlagM;
  x = u & SNES::CPUDebugger::UsageFlagX;

  switch (source) {
  case CPU:
    if (!u) {
      e = SNES::cpu.regs.e;
      m = SNES::cpu.regs.p.m;
      x = SNES::cpu.regs.p.x;
    }
    SNES::cpu.disassemble_opcode_ex(opcode, address, e, m, x);
    break;

  case SA1:
    if (!u) {
      e = SNES::sa1.regs.e;
      m = SNES::sa1.regs.p.m;
      x = SNES::sa1.regs.p.x;
    }
    SNES::sa1.disassemble_opcode_ex(opcode, address, e, m, x);
    break;
  }

  result.setOpcode(address, opcode.opcode);
  setOpcodeParams(result, opcode, address);

  if (opcode.isBra() || opcode.isBraWithContinue()) {
    result.setBra(decode(opcode.optype, opcode.opall(), address));
  }
  if (opcode.returns()) {
    result.flags |= DisassemblerLine::FLAG_RETURN;
  }

  // Advance to next
  if ((usagePointer[(address + opcode.size()) & 0xFFFFFF] & SNES::CPUDebugger::UsageOpcode) != 0) {
    address += opcode.size();
  }

  return true;
}

// ------------------------------------------------------------------------
void CpuDisasmProcessor::analyze(uint32_t address) {
  // TODO: support this for SA1 as well
  if (source != CPU) return;

  uint8_t u = usage(address);
  bool e, m, x;

  if (!u) {
    e = SNES::cpu.regs.e;
    m = SNES::cpu.regs.p.m;
    x = SNES::cpu.regs.p.x;
  } else {
    e = u & SNES::CPUDebugger::UsageFlagE;
    m = u & SNES::CPUDebugger::UsageFlagM;
    x = u & SNES::CPUDebugger::UsageFlagX;
  }

  SNES::CPUAnalystState state(e, m, x);
  SNES::cpuAnalyst.performAnalysis(address, state, true);
}

// ------------------------------------------------------------------------
void CpuDisasmProcessor::findKnownRange(uint32_t currentAddress, uint32_t &startAddress, uint32_t &endAddress, uint32_t &currentAddressLine, uint32_t &numLines) {
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
      if ((usagePointer[(startAddress - i) & 0xFFFFFF] & SNES::CPUDebugger::UsageOpcode) == 0) {
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
      if ((usagePointer[(endAddress + i) & 0xFFFFFF] & SNES::CPUDebugger::UsageOpcode) == 0) {
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
uint8_t CpuDisasmProcessor::usage(uint32_t address) {
  return usagePointer[address & 0xFFFFFF];
}

// ------------------------------------------------------------------------
uint8_t CpuDisasmProcessor::read(uint32_t address) {
  if (!SNES::cartridge.loaded()) {
    return 0;
  }

  SNES::debugger.bus_access = true;
  uint8_t data = SNES::debugger.read(SNES::Debugger::MemorySource::CPUBus, address & 0xFFFFFF);
  SNES::debugger.bus_access = false;
  return data;
}

// ------------------------------------------------------------------------
void CpuDisasmProcessor::write(uint32_t address, uint8_t data) {
  if (!SNES::cartridge.loaded()) {
    return;
  }

  SNES::debugger.bus_access = true;
  SNES::debugger.write(SNES::Debugger::MemorySource::CPUBus, address & 0xFFFFFF, data);
  SNES::debugger.bus_access = false;
}

// ------------------------------------------------------------------------
uint32_t CpuDisasmProcessor::getBusSize() {
  return 0x1000000;
}
