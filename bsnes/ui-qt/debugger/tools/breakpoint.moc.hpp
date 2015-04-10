class BreakpointItem : public QWidget {
  Q_OBJECT

enum {
  BreakEnable,
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
  QCheckBox *enabled;
  QLineEdit *addr;
  QLineEdit *addr_end;
  QLineEdit *data;
  QCheckBox *mode_r;
  QCheckBox *mode_w;
  QCheckBox *mode_x;
  QComboBox *source;
  BreakpointItem(unsigned id);

public slots:
  void toggle();

private:
  const unsigned id;
};

class BreakpointEditor : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  BreakpointItem *breakpoint[SNES::Debugger::Breakpoints];

  BreakpointEditor();
};

extern BreakpointEditor *breakpointEditor;
