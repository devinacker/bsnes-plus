class MemHexEditor : public HexEditor {
  Q_OBJECT
  
public:
  function<uint8_t (unsigned)> usage;
  enum Usage {
    UsageRead  = 0x80,
    UsageWrite = 0x40,
    UsageExec  = 0x20,
  };
  
  virtual void refresh();
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
  void exportMemory();
  void importMemory();
  void exportMemory(SNES::Memory&, const string&) const;
  void importMemory(SNES::Memory&, const string&) const;
};

extern MemoryEditor *memoryEditor;
