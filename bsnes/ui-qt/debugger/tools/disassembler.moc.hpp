class DisasmWidget : public QWidget {
public:
  QVBoxLayout *layout;
  QTextEdit *view;
  DisasmWidget();
};

class Disassembler : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tab;

  enum Source { CPU, SMP, SA1 };
  void refresh(Source, unsigned);
  Disassembler();
};

extern DisasmWidget *cpuDisassembler;
extern DisasmWidget *smpDisassembler;
extern DisasmWidget *sa1Disassembler;
extern Disassembler *disassembler;
