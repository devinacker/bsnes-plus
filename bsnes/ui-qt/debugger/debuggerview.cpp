#include "disassembler/symbolsview.cpp"
#include "disassembler/disassemblerview.cpp"
#include "debuggerview.moc"

DebuggerView::DebuggerView(RegisterEdit *registers, DisasmProcessor *processor, bool step) : registers(registers), processor(processor) {
  addressOffset = 0;
  cursorPosition = 0;

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  consoleLayout = new QSplitter(Qt::Vertical);
  consoleLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(consoleLayout);

  disassembler = new DisassemblerView(processor);
  disassembler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  consoleLayout->addWidget(disassembler);

  ramViewer = new QHexEdit;
  ramViewer->reader = { &DebuggerView::reader, this };
  ramViewer->writer = { &DebuggerView::writer, this };
  ramViewer->usage  = { &DebuggerView::usage, this };
  // set fixed address width based on address bus size
  ramViewer->setEditorSize(processor->getBusSize());
  ramViewer->setAddressWidth(ramViewer->addressWidth());
  ramViewer->setMinimumHeight(70);
  ramViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  ramViewer->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  consoleLayout->addWidget(ramViewer);

  disassembler->setAddressWidth(ramViewer->addressWidth());

  consoleLayout->setStretchFactor(0, 8);
  consoleLayout->setStretchFactor(1, 1);

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

  if (processor->getSymbols() != NULL) {
    symbolsViewerDialog = new SymbolsView(processor);
    symbolsViewer = new QPushButton("Symbols");
    controlLayout->addWidget(symbolsViewer);
    connect(symbolsViewer, SIGNAL(clicked(bool)), symbolsViewerDialog, SLOT(show()));
  }

  controlLayout->addStretch();

  connect(stepProcessor, SIGNAL(clicked(bool)), this, SLOT(synchronize()));
  connect(traceProcessor, SIGNAL(stateChanged(int)), this, SIGNAL(traceStateChanged(int)));

  connect(consoleLayout, SIGNAL(splitterMoved(int,int)), this, SLOT(synchronize()));

  synchronize();
}

// ------------------------------------------------------------------------
uint8_t DebuggerView::reader(unsigned addr) {
  return processor->read(addr + addressOffset);
}

// ------------------------------------------------------------------------
void DebuggerView::writer(unsigned addr, uint8_t data) {
  processor->write(addr + addressOffset, data);
}

// ------------------------------------------------------------------------
uint8_t DebuggerView::usage(unsigned addr) {
  return processor->usage(addr + addressOffset);
}

// ------------------------------------------------------------------------
void DebuggerView::refresh(uint32_t address) {
  disassembler->refresh(address);

  uint32_t getAddress = address;
  DisassemblerLine line;
  processor->getLine(line, getAddress);

  if (line.hasAddress() && !line.isBra()) {
    for (uint32_t i=0; i<line.params.size(); i++) {
      const DisassemblerParam &param = line.params[i];

      if (param.type == DisassemblerParam::Address) {
        cursorPosition = (param.address & 0xF) * 2;
        addressOffset  = param.address & ~0xF;
      }
    }
  }

  ramViewer->setAddressOffset(addressOffset);
  ramViewer->setCursorPosition(cursorPosition);
  ramViewer->refresh(true);
}

// ------------------------------------------------------------------------
void DebuggerView::synchronize() {
  int h = ceil((double)ramViewer->height() / (ramViewer->fontMetrics().height() + 1)) - 1;
  if (h < 1) { h = 1; }

  ramViewer->setEditorSize((uint32_t)h * 0x10);
  ramViewer->verticalScrollBar()->setMaximum(0);

  emit synchronized();
}

// ------------------------------------------------------------------------
void DebuggerView::resizeEvent(QResizeEvent *ev) {
  QTimer::singleShot(0, this, SLOT(synchronize()));
  QWidget::resizeEvent(ev);
}
