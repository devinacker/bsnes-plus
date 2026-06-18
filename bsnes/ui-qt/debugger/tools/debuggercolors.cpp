#include "debuggercolors.moc"
#include "colorpickerbutton.moc"

#include "colorpickerbutton/colorpickerbutton.cpp"

DebuggerColorsWindow *debuggerColors;

DebuggerColorsWindow::DebuggerColorsWindow() {
  setObjectName("debugger-colors");
  setWindowTitle("Debugger Colors");
  setGeometryString(&config().geometry.debuggerColors);
  setMinimumWidth(300);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  disassemblerLabel = new QLabel("<b>Disassembler</b>");
  layout->addWidget(disassemblerLabel);

  disassemblerLayout = new QGridLayout;
  layout->addLayout(disassemblerLayout);

  disassemblerOpLabel = new QLabel("Opcode:");
  disassemblerLayout->addWidget(disassemblerOpLabel, 0, 0);
  disassemblerOpPicker = new ColorPickerButton;
  disassemblerLayout->addWidget(disassemblerOpPicker, 0, 1);
  
  disassemblerParamImmediateLabel = new QLabel("Immediate parameter:");
  disassemblerLayout->addWidget(disassemblerParamImmediateLabel, 1, 0);
  disassemblerParamImmediatePicker = new ColorPickerButton;
  disassemblerLayout->addWidget(disassemblerParamImmediatePicker, 1, 1);
  
  disassemblerParamAddressLabel = new QLabel("Address parameter:");
  disassemblerLayout->addWidget(disassemblerParamAddressLabel, 2, 0);
  disassemblerParamAddressPicker = new ColorPickerButton;
  disassemblerLayout->addWidget(disassemblerParamAddressPicker, 2, 1);
  
  disassemblerParamSymbolLabel = new QLabel("Symbol parameter:");
  disassemblerLayout->addWidget(disassemblerParamSymbolLabel, 3, 0);
  disassemblerParamSymbolPicker = new ColorPickerButton;
  disassemblerLayout->addWidget(disassemblerParamSymbolPicker, 3, 1);

  consoleLabel = new QLabel("<b>Console</b>");
  layout->addWidget(consoleLabel);

  consoleLayout = new QGridLayout;
  layout->addLayout(consoleLayout);

  consoleBreakpointLabel = new QLabel("Breakpoint:");
  consoleLayout->addWidget(consoleBreakpointLabel, 0, 0);
  consoleBreakpointPicker = new ColorPickerButton;
  consoleLayout->addWidget(consoleBreakpointPicker, 0, 1);

  consoleCpuStepLabel = new QLabel("CPU step:");
  consoleLayout->addWidget(consoleCpuStepLabel, 1, 0);
  consoleCpuStepPicker = new ColorPickerButton;
  consoleLayout->addWidget(consoleCpuStepPicker, 1, 1);

  consoleSmpStepLabel = new QLabel("SMP step:");
  consoleLayout->addWidget(consoleSmpStepLabel, 2, 0);
  consoleSmpStepPicker = new ColorPickerButton;
  consoleLayout->addWidget(consoleSmpStepPicker, 2, 1);

  consoleCoprocessorStepLabel = new QLabel("Coprocessor step:");
  consoleLayout->addWidget(consoleCoprocessorStepLabel, 3, 0);
  consoleCoprocessorStepPicker = new ColorPickerButton;
  consoleLayout->addWidget(consoleCoprocessorStepPicker, 3, 1);

  hexEditorLabel = new QLabel("<b>Hex Editor</b>");
  layout->addWidget(hexEditorLabel);

  hexEditorLayout = new QGridLayout;
  layout->addLayout(hexEditorLayout);

  hexEditorReadUsageLabel = new QLabel("Read usage:");
  hexEditorLayout->addWidget(hexEditorReadUsageLabel, 0, 0);
  hexEditorReadUsagePicker = new ColorPickerButton;
  hexEditorLayout->addWidget(hexEditorReadUsagePicker, 0, 1);

  hexEditorWriteUsageLabel = new QLabel("Write usage:");
  hexEditorLayout->addWidget(hexEditorWriteUsageLabel, 1, 0);
  hexEditorWriteUsagePicker = new ColorPickerButton;
  hexEditorLayout->addWidget(hexEditorWriteUsagePicker, 1, 1);

  hexEditorExecUsageLabel = new QLabel("Exec usage:");
  hexEditorLayout->addWidget(hexEditorExecUsageLabel, 2, 0);
  hexEditorExecUsagePicker = new ColorPickerButton;
  hexEditorLayout->addWidget(hexEditorExecUsagePicker, 2, 1);

  hexEditorRWEUsageLabel = new QLabel("R/W/E usage:");
  hexEditorLayout->addWidget(hexEditorRWEUsageLabel, 3, 0);
  hexEditorRWEUsagePicker = new ColorPickerButton;
  hexEditorLayout->addWidget(hexEditorRWEUsagePicker, 3, 1);

  initializeUi();

  connect(disassemblerOpPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(setDisassemblerOpColor(const QColor &)));
  connect(disassemblerParamImmediatePicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(setDisassemblerParamImmediateColor(const QColor &)));
  connect(disassemblerParamAddressPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(setDisassemblerParamAddressColor(const QColor &)));
  connect(disassemblerParamSymbolPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(setDisassemblerParamSymbolColor(const QColor &)));
  connect(hexEditorReadUsagePicker, SIGNAL(colorChanged(const QColor &)), this, SIGNAL(hexEditorColorChanged()));
  connect(hexEditorWriteUsagePicker, SIGNAL(colorChanged(const QColor &)), this, SIGNAL(hexEditorColorChanged()));
  connect(hexEditorExecUsagePicker, SIGNAL(colorChanged(const QColor &)), this, SIGNAL(hexEditorColorChanged()));
  connect(hexEditorRWEUsagePicker, SIGNAL(colorChanged(const QColor &)), this, SIGNAL(hexEditorColorChanged()));
}

void DebuggerColorsWindow::initializeUi() {
  disassemblerOpPicker->setColor(config().debugger.disassembler.opColor());
  disassemblerParamImmediatePicker->setColor(config().debugger.disassembler.paramImmediateColor());
  disassemblerParamAddressPicker->setColor(config().debugger.disassembler.paramAddressColor());
  disassemblerParamSymbolPicker->setColor(config().debugger.disassembler.paramSymbolColor());
  consoleBreakpointPicker->setColor(config().debugger.console.breakpointColor());
  consoleCpuStepPicker->setColor(config().debugger.console.cpuStepColor());
  consoleSmpStepPicker->setColor(config().debugger.console.smpStepColor());
  consoleCoprocessorStepPicker->setColor(config().debugger.console.coprocessorStepColor());
  hexEditorReadUsagePicker->setColor(config().debugger.hexEditor.readUsageColor());
  hexEditorWriteUsagePicker->setColor(config().debugger.hexEditor.writeUsageColor());
  hexEditorExecUsagePicker->setColor(config().debugger.hexEditor.execUsageColor());
  hexEditorRWEUsagePicker->setColor(config().debugger.hexEditor.rweUsageColor());
}

void DebuggerColorsWindow::setDisassemblerOpColor(const QColor &color) {
  config().debugger.disassembler.opColor = color.name();
  emit disassemblerColorChanged();
}

void DebuggerColorsWindow::setDisassemblerParamImmediateColor(const QColor &color) {
  config().debugger.disassembler.paramImmediateColor = color.name();
  emit disassemblerColorChanged();
}

void DebuggerColorsWindow::setDisassemblerParamAddressColor(const QColor &color) {
  config().debugger.disassembler.paramAddressColor = color.name();
  emit disassemblerColorChanged();
}

void DebuggerColorsWindow::setDisassemblerParamSymbolColor(const QColor &color) {
  config().debugger.disassembler.paramSymbolColor = color.name();
  emit disassemblerColorChanged();
}

void DebuggerColorsWindow::setConsoleBreakpointColor(const QColor &color) {
  config().debugger.console.breakpointColor = color.name();
}

void DebuggerColorsWindow::setConsoleCpuStepColor(const QColor &color) {
  config().debugger.console.cpuStepColor = color.name();
}

void DebuggerColorsWindow::setConsoleSmpStepColor(const QColor &color) {
  config().debugger.console.smpStepColor = color.name();
}

void DebuggerColorsWindow::setConsoleCoprocessorStepColor(const QColor &color) {
  config().debugger.console.coprocessorStepColor = color.name();
}

void DebuggerColorsWindow::setHexEditorReadUsageColor(const QColor &color) {
  config().debugger.hexEditor.readUsageColor = color.name();
  emit hexEditorColorChanged();
}

void DebuggerColorsWindow::setHexEditorWriteUsageColor(const QColor &color) {
  config().debugger.hexEditor.writeUsageColor = color.name();
  emit hexEditorColorChanged();
}

void DebuggerColorsWindow::setHexEditorExecUsageColor(const QColor &color) {
  config().debugger.hexEditor.execUsageColor = color.name();
  emit hexEditorColorChanged();
}
 
void DebuggerColorsWindow::setHexEditorRWEUsageColor(const QColor &color) {
  config().debugger.hexEditor.rweUsageColor = color.name();
  emit hexEditorColorChanged();
}