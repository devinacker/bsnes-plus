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
      if ((SNES::smp.usage[(currentAddress + i) & 0xFFFF] & SNES::SMPDebugger::UsageOpcode) == 0) {
        continue;
      }

      currentAddress += i;
      break;
    }
  }

  return currentAddress;
}

// ------------------------------------------------------------------------
uint32_t SmpDisasmProcessor::decode(uint32_t type, uint32_t address, uint32_t pc) {
  return SNES::smp.decode(type, address, pc);
}

// ------------------------------------------------------------------------
void SmpDisasmProcessor::setOpcodeParams(DisassemblerLine &result, SNES::SMP::Opcode &opcode, uint32_t pc) {
  uint16_t directAddr = decode(SNESSMP::Direct, opcode.op8(), pc);
  DisassemblerParam directParam = DisassemblerParam::createAddress(opcode.op8(), directAddr);

  uint16_t absoluteAddr = opcode.op16();
  DisassemblerParam absoluteParam = DisassemblerParam::createAddress(absoluteAddr, absoluteAddr);

  switch (opcode.optype) {
    case SNESSMP::Implied:
      result.paramFormat = "";
      break;
      
    case SNESSMP::Direct:
      result.paramFormat = "%1X2";
      result.params.append(directParam);
      break;

    case SNESSMP::DirectRelative:
      result.paramFormat = "%1X2, %2X4";
      result.params.append(directParam);
      result.params.append(DisassemblerParam::createAddress(decode(opcode.optype, opcode.op8(1), pc), 
                             decode(opcode.optype, opcode.op8(1), pc)));
      break;

    case SNESSMP::DirectDirect:
      result.paramFormat = "%1X2, %2X2";
      result.params.append(DisassemblerParam::createAddress(opcode.op8(1),
                             decode(SNESSMP::Direct, opcode.op8(1), pc)));
      result.params.append(directParam);
      break;
    
    case SNESSMP::DirectConstant:
      result.paramFormat = "%1X2, #%2X2";
      result.params.append(DisassemblerParam::createAddress(opcode.op8(1),
                             decode(SNESSMP::Direct, opcode.op8(1), pc)));
      result.params.append(DisassemblerParam::createValue(opcode.op8(0)));
      break;

    case SNESSMP::DirectIndX:
      result.paramFormat = "%1X2+x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESSMP::DirectA:
      result.paramFormat = "%1X2, a";
      result.params.append(directParam);
      break;
    
    case SNESSMP::DirectY:
      result.paramFormat = "%1X2, y";
      result.params.append(directParam);
      break;
    
    case SNESSMP::DirectIndXA:
      result.paramFormat = "%1X2+x, a";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
    
    case SNESSMP::DirectX:
      result.paramFormat = "%1X2, x";
      result.params.append(directParam);
      break;
      
    case SNESSMP::DirectIndYX:
      result.paramFormat = "%1X2+y, x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
    
    case SNESSMP::DirectYa:
      result.paramFormat = "%1X2, ya";
      result.params.append(directParam);
      break;
      
    case SNESSMP::DirectIndXY:
      result.paramFormat = "%1X2+x, y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
    
    case SNESSMP::DirectIndXRelative:
      result.paramFormat = "%1X2+x, %2X4";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(SNESSMP::DirectIndX, opcode.op8(), pc)));
      result.params.append(DisassemblerParam::createAddress(decode(opcode.optype, opcode.op8(1), pc), 
                             decode(opcode.optype, opcode.op8(1), pc)));
      break;
      
    case SNESSMP::Absolute:
      result.paramFormat = "%1X4";
      result.params.append(absoluteParam);
      break;
      
    case SNESSMP::AbsoluteA:
      result.paramFormat = "%1X4, a";
      result.params.append(absoluteParam);
      break;
      
    case SNESSMP::AbsoluteX:
      result.paramFormat = "%1X4, x";
      result.params.append(absoluteParam);
      break;
      
    case SNESSMP::AbsoluteBitC:
      result.paramFormat = "%1X4:";
      result.paramFormat << (opcode.op16() >> 13) << ", C";
      result.params.append(DisassemblerParam::createAddress(opcode.op16() & 0x1fff, 
                             decode(SNESSMP::Absolute, opcode.op16() & 0x1fff, pc)));
      break;
    
    case SNESSMP::AbsoluteY:
      result.paramFormat = "%1X4, y";
      result.params.append(absoluteParam);
      break;
    
    case SNESSMP::AbsoluteXA:
      result.paramFormat = "%1X4+x, a";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             absoluteAddr, decode(opcode.optype, opcode.op16(), pc)));
      break;
      	
    case SNESSMP::AbsoluteYA:
      result.paramFormat = "%1X4+y, a";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             absoluteAddr, decode(opcode.optype, opcode.op16(), pc)));
      break;
    
    case SNESSMP::ADirect:
      result.paramFormat = "a, %1X2";
      result.params.append(directParam);
      break;

    case SNESSMP::AAbsolute:
      result.paramFormat = "a, %1X4";
      result.params.append(absoluteParam);
      break;
    
    case SNESSMP::AIDirectIndX:
      result.paramFormat = "a, (%1X2+x)";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESSMP::AConstant:
      result.paramFormat = "a, #%1X2";
      result.params.append(DisassemblerParam::createValue(opcode.op8()));
      break;
    
    case SNESSMP::CAbsoluteBit:
      result.paramFormat = "c, %1X4:";
      result.paramFormat << (opcode.op16() >> 13);
      result.params.append(DisassemblerParam::createAddress(opcode.op16() & 0x1fff, 
                             decode(SNESSMP::Absolute, opcode.op16() & 0x1fff, pc)));
      break;
    
    case SNESSMP::CNAbsoluteBit:
      result.paramFormat = "c, !%1X4:";
      result.paramFormat << (opcode.op16() >> 13);
      result.params.append(DisassemblerParam::createAddress(opcode.op16() & 0x1fff, 
                             decode(SNESSMP::Absolute, opcode.op16() & 0x1fff, pc)));
      break;
    
    case SNESSMP::Relative:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress(decode(opcode.optype, opcode.op8(), pc), 
                             decode(opcode.optype, opcode.op8(), pc)));
      break;
      
    case SNESSMP::ADirectIndX:
      result.paramFormat = "a, %1X2+x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;

    case SNESSMP::AAbsoluteX:
      result.paramFormat = "a, %1X4+x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             absoluteAddr, decode(opcode.optype, opcode.op16(), pc)));
      break;
      
    case SNESSMP::AAbsoluteY:
      result.paramFormat = "a, %1X4+y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             absoluteAddr, decode(opcode.optype, opcode.op16(), pc)));
      break;
      
    case SNESSMP::AIDirectIndY:
      result.paramFormat = "a, (%1X2)+y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
    
    case SNESSMP::XAbsolute:
      result.paramFormat = "x, %1X4";
      result.params.append(absoluteParam);
      break;
    
    case SNESSMP::IAbsoluteX:
      result.paramFormat = "(%1X4+x)";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op16(),
                             absoluteAddr, decode(opcode.optype, opcode.op16(), pc)));
      break;
    
    case SNESSMP::IDirectIndXA:
      result.paramFormat = "(%1X2+x), a";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
      
    case SNESSMP::IDirectIndYA:
      result.paramFormat = "(%1X2)+y, a";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
        
    case SNESSMP::XDirect:
      result.paramFormat = "x, %1X2";
      result.params.append(directParam);
      break;
      
    case SNESSMP::XDirectIndY:
      result.paramFormat = "x, %1X2+y";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
    
  	case SNESSMP::XConstant:
      result.paramFormat = "x, #%1X2";
      result.params.append(DisassemblerParam::createValue(opcode.op8()));
      break;
    
    case SNESSMP::PVector:
      result.paramFormat = "%1X4";
      result.params.append(DisassemblerParam::createAddress(opcode.op8(),
                             decode(opcode.optype, opcode.op8(), pc)));
      break;
      
    case SNESSMP::TVector:
      result.paramFormat = string() << (opcode.op() >> 4);
      break;
    
    case SNESSMP::YaDirect:
      result.paramFormat = "ya, %1X2";
      result.params.append(directParam);
      break;
      
    case SNESSMP::YAbsolute:
      result.paramFormat = "y, %1X4";
      result.params.append(absoluteParam);
      break;
    
    case SNESSMP::YDirect:
      result.paramFormat = "y, %1X2";
      result.params.append(directParam);
      break;
    
    case SNESSMP::YConstant:
      result.paramFormat = "y, #%1X2";
      result.params.append(DisassemblerParam::createValue(opcode.op8()));
      break;
    
    case SNESSMP::YDirectIndX:
      result.paramFormat = "y, %1X2+x";
      result.params.append(DisassemblerParam::createTargetAddress(opcode.op8(),
                             directAddr, decode(opcode.optype, opcode.op8(), pc)));
      break;
      
    case SNESSMP::YRelative:
      result.paramFormat = "y, %1X4";
      result.params.append(DisassemblerParam::createAddress(decode(opcode.optype, opcode.op8(), pc), 
                             decode(opcode.optype, opcode.op8(), pc)));
      break;
      
    case SNESSMP::A:
      result.paramFormat = "a";
      break;

    case SNESSMP::AX:
      result.paramFormat = "a, x";
      break;
    
    case SNESSMP::AIX:
      result.paramFormat = "a, (x)";
      break;
      
    case SNESSMP::P:
      result.paramFormat = "p";
      break;
    
    case SNESSMP::IXIY:
      result.paramFormat = "(x), (y)";
      break;
      
    case SNESSMP::X:
      result.paramFormat = "x";
      break;
      
    case SNESSMP::XA:
      result.paramFormat = "x, a";
      break;
      
    case SNESSMP::XSp:
      result.paramFormat = "x, sp";
      break;
    
    case SNESSMP::Y:
      result.paramFormat = "y";
      break;
      
    case SNESSMP::YaX:
      result.paramFormat = "ya, x";
      break;
      
    case SNESSMP::IXPA:
      result.paramFormat = "(x)+, a";
      break;
    
    case SNESSMP::SpX:
      result.paramFormat = "sp, x";
      break;
    
    case SNESSMP::AIXP:
      result.paramFormat = "a, (x)+";
      break;
      
    case SNESSMP::IXA:
      result.paramFormat = "(x), a";
      break;
    
    case SNESSMP::Ya:
      result.paramFormat = "ya";
      break;
    
    case SNESSMP::AY:
      result.paramFormat = "a, y";
      break;
    
    case SNESSMP::YA:
      result.paramFormat = "y, a";
      break;
    
    default: result.paramFormat = "???"; break;
  }
}

// ------------------------------------------------------------------------
bool SmpDisasmProcessor::getLine(DisassemblerLine &result, uint32_t &address) {
  SNES::SMP::Opcode opcode;
  
  SNES::smp.disassemble_opcode_ex(opcode, address);

  result.setOpcode(address, opcode.opcode);
  setOpcodeParams(result, opcode, address);
  
  if (opcode.isBra() || opcode.isBraWithContinue()) {
    result.setBra(decode(opcode.optype, opcode.opall(), address));
  }
  if (opcode.returns()) {
    result.flags |= DisassemblerLine::FLAG_RETURN;
  }
  
  // Advance to next
  for (uint32_t i=1; i<=4; i++) {
    if ((SNES::smp.usage[(address + i) & 0xFFFF] & SNES::SMPDebugger::UsageOpcode) == 0) {
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
      if ((SNES::smp.usage[(startAddress - i) & 0xFFFF] & SNES::SMPDebugger::UsageOpcode) == 0) {
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
      if ((SNES::smp.usage[(endAddress + i) & 0xFFFF] & SNES::SMPDebugger::UsageOpcode) == 0) {
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
