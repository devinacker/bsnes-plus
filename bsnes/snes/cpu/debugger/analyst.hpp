struct CPUAnalystState {
  CPUAnalystState(bool e=true, bool m=true, bool x=true) : e(e), m(m), x(x), bank(0) {}

  inline unsigned char mask() const {
    return
        (e ? CPUDebugger::UsageFlagE : 0)
      | (m ? CPUDebugger::UsageFlagM : 0)
      | (x ? CPUDebugger::UsageFlagX : 0)
    ;
  }

  unsigned char bank;
  bool e;
  bool m;
  bool x;
};

class CPUAnalyst {
public:
  void reset();
  void performFullAnalysis();
  void performAnalysisForVector(uint32_t address, bool emulation=false);
  uint32_t performAnalysis(uint32_t address, const CPUAnalystState &state, bool force=false);
};
