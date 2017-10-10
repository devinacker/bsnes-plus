#include "disassembler/disassemblerview.cpp"
#include "debuggerview.moc"

DebuggerView::DebuggerView(RegisterEdit *registers, DisasmProcessor *processor, bool step) : registers(registers) {
  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  consoleLayout = new QVBoxLayout;
  consoleLayout->setSpacing(0);
  layout->addLayout(consoleLayout);

  disassembler = new DisassemblerView(processor);
  disassembler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  consoleLayout->addWidget(disassembler);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  registers->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  controlLayout->addWidget(registers);

  QFrame *line;
  line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  controlLayout->addWidget(line);

  stepProcessor = new QCheckBox("Step");
  stepProcessor->setChecked(step);
  controlLayout->addWidget(stepProcessor);

  traceProcessor = new QCheckBox("Trace");
  controlLayout->addWidget(traceProcessor);

  controlLayout->addStretch();

  connect(stepProcessor, SIGNAL(released()), this, SLOT(synchronize()));
  connect(traceProcessor, SIGNAL(stateChanged(int)), this, SIGNAL(traceStateChanged(int)));

  synchronize();
}

void DebuggerView::refresh(uint32_t address) {
  disassembler->refresh(address);
}

void DebuggerView::synchronize() {

  emit synchronized();
}
