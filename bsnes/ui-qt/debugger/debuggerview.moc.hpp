class DebuggerView : public QWidget {
  Q_OBJECT

public:
  DebuggerView(class RegisterEdit *registers, class DisasmProcessor *processor, bool step=false);

  QHBoxLayout *layout;
  QSplitter *consoleLayout;
  QVBoxLayout *controlLayout;
  QCheckBox *stepProcessor;
  QCheckBox *traceProcessor;
  QPushButton *symbolsViewer;
  class QHexEdit *ramViewer;
  class SymbolsView *symbolsViewerDialog;
  class DisassemblerView *disassembler;
  class DisasmProcessor *processor;

  void refresh(uint32_t address);
  uint8_t reader(unsigned addr);
  void writer(unsigned addr, uint8_t data);
  uint8_t usage(unsigned addr);

public slots:
  void synchronize();

signals:
  void synchronized();
  void traceStateChanged(int);

private:
  class RegisterEdit *registers;
  uint32_t addressOffset;
  uint32_t cursorPosition;
};
