class BreakpointItem : public QWidget {
  Q_OBJECT

enum {
  BreakAddrStart,
  BreakAddrDash,
  BreakAddrEnd,
  BreakData,
  BreakRead,
  BreakWrite,
  BreakExecute,
  BreakSource
};

public:
  QGridLayout *layout;
  QLineEdit *addr;
  QLineEdit *addr_end;
  QLineEdit *data;
  QCheckBox *mode_r;
  QCheckBox *mode_w;
  QCheckBox *mode_x;
  QComboBox *source;
  BreakpointItem(unsigned id);

  void removeBreakpoint();
  void setBreakpoint(string addr, string mode, string source);
  string toString() const;

  bool isEnabled() const;
  uint32_t getAddressFrom() const;
  uint32_t getAddressTo() const;
  bool isModeR() const;
  bool isModeW() const;
  bool isModeX() const;
  string getBus() const;

public slots:
  void init();
  void toggle();
  void clear();

private:
  const unsigned id;
};

class BreakpointEditor : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  BreakpointItem *breakpoint[SNES::Debugger::Breakpoints];
  QCheckBox *breakOnWDM;
  QCheckBox *breakOnBRK;

  BreakpointEditor();

  void addBreakpoint(const string& addr, const string& mode, const string& source);
  void addBreakpoint(const string& breakpoint);
  void removeBreakpoint(uint32_t index);
  void setBreakOnBrk(bool b);
  string toStrings() const;

  int32_t indexOfBreakpointExec(uint32_t addr, const string &source) const;

public slots:
  void toggle();
  void clear();
};

extern BreakpointEditor *breakpointEditor;
