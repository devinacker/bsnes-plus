class MemHexEditor : public HexEditor {
  Q_OBJECT
  
public:
  function<uint8_t (unsigned)> usage;
  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
  };
  
  void refresh();
  unsigned offset() const { return editorOffset; };
  unsigned size() const { return editorSize; };
};

class MemoryEditor : public Window {
  Q_OBJECT

public:
  QHBoxLayout *layout;
  MemHexEditor *editor;
  QVBoxLayout *controlLayout;
  QComboBox *source;
  QLineEdit *addr;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  
  QHBoxLayout *toolLayout;
  QToolButton *prevCodeButton;
  QToolButton *nextCodeButton;
  QToolButton *prevDataButton;
  QToolButton *nextDataButton;
  QToolButton *prevUnkButton;
  QToolButton *nextUnkButton;
  
  QWidget *spacer;
  QPushButton *exportButton;
  QPushButton *importButton;

  void autoUpdate();
  void synchronize();

  SNES::Debugger::MemorySource memorySource;
  uint8_t reader(unsigned addr);
  void writer(unsigned addr, uint8_t data);
  uint8_t usage(unsigned addr);

  MemoryEditor();

public slots:
  void show();
  void sourceChanged(int);
  void refresh();
  void updateOffset();
  
  void prevCode();
  void nextCode();
  void prevData();
  void nextData();
  void prevUnknown();
  void nextUnknown();
  void gotoPrevious(int);
  void gotoNext(int);
  
  void exportMemory();
  void importMemory();
  void exportMemory(SNES::Memory&, const string&) const;
  void importMemory(SNES::Memory&, const string&) const;
};

extern MemoryEditor *memoryEditor;
