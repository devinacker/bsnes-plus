#ifdef CPU_CPP

// ------------------------------------------------------------------------
void CPUAnalyst::reset() {
  uint32_t size = 1 << 24;
  for (uint32_t i=0; i<size; i++) {
    cpu.usage[i] = 0;
    cpu.cart_usage[i] = 0;
  }
}

// ------------------------------------------------------------------------
void CPUAnalyst::performFullAnalysis() {
  reset();

  performAnalysisForVector(0xFFE4);
  performAnalysisForVector(0xFFE6);
  performAnalysisForVector(0xFFE8);
  performAnalysisForVector(0xFFEA);
  performAnalysisForVector(0xFFEC);
  performAnalysisForVector(0xFFEE);

  performAnalysisForVector(0xFFF4, true);
  performAnalysisForVector(0xFFF6, true);
  performAnalysisForVector(0xFFF8, true);
  performAnalysisForVector(0xFFFA, true);
  performAnalysisForVector(0xFFFC, true);
  performAnalysisForVector(0xFFFE, true);
}

// ------------------------------------------------------------------------
void CPUAnalyst::performAnalysisForVector(uint32_t address, bool emulation) {
  CPUAnalystState state(emulation);

  uint32_t numRoutines = performAnalysis(bus.read(address) | bus.read(address + 1) << 8, state);

  puts(string("Found ", numRoutines, " new symbols at vector $", hex<6,'0'>(address)));
}

// ------------------------------------------------------------------------
uint32_t CPUAnalyst::performAnalysis(uint32_t address, const CPUAnalystState &_state, bool force) {
  if (cpu.usage[address] != 0 && !force) {
    return 0;
  }

  CPUAnalystState state = _state;
  CPUDebugger::Opcode op;
  linear_vector<CPUAnalystState> stackP;
  uint32_t maxMethodSize = 0x1000;
  uint32_t numRoutines = 1;

  while (--maxMethodSize) {
    address = address & 0xFFFFFF;
    if (cpu.usage[address] != 0 && !force) {
      break;
    }

    cpu.usage[address] |= CPUDebugger::UsageExec | CPUDebugger::UsageOpcode | state.mask();
    cpu.disassemble_opcode_ex(op, address, state.e, state.m, state.x);

    if (op.setsX()) { state.x = true; }
    if (op.setsM()) { state.m = true; }
    if (op.resetsX()) { state.x = false; }
    if (op.resetsM()) { state.m = false; }
    if (op.resetsE()) { state.e = false; }

    if (op.pushesP()) {
      stackP.append(state);
    }
    if (op.popsP() && stackP.size()) {
      uint32_t index = stackP.size() - 1;
      state = stackP[index];
      stackP.remove(index);
    }

    if (op.breaks()) {
      break;
    }

    if (op.isBraWithContinue()) {
      numRoutines += performAnalysis(cpu.decode(op.optype, op.opall(), address), state);
    }

    if (op.returns()) {
      break;
    } else if (op.isBra()) {
      address = cpu.decode(op.optype, op.opall(), address);
      numRoutines++;
    } else {
      address += op.size();
    }
  }

  return numRoutines;
}

#endif
