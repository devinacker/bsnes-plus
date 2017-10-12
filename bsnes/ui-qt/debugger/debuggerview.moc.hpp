class DebuggerView : public QWidget {
  Q_OBJECT

public:
  DebuggerView(class RegisterEdit *registers, class DisasmProcessor *processor, bool step=false);

  QHBoxLayout *layout;
  QVBoxLayout *consoleLayout;
  QVBoxLayout *controlLayout;
  QCheckBox *stepProcessor;
  QCheckBox *traceProcessor;
  QPushButton *symbolsViewer;
  class SymbolsView *symbolsViewerDialog;
  class DisassemblerView *disassembler;

  void refresh(uint32_t address);

public slots:
  void synchronize();

signals:
  void synchronized();
  void traceStateChanged(int);

private:
  class RegisterEdit *registers;
};
