#include "breakpoint.moc"
BreakpointEditor *breakpointEditor;

BreakpointItem::BreakpointItem(unsigned id_) : id(id_) {
  layout = new QGridLayout;
  layout->setMargin(0);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  enabled = new QCheckBox;
  layout->addWidget(enabled, 1, BreakEnable);

  addr = new QLineEdit;
  addr->setFixedWidth(80);
  layout->addWidget(addr, 1, BreakAddrStart);

  layout->addWidget(new QLabel(" - "), 1, BreakAddrDash);

  addr_end = new QLineEdit;
  addr_end->setFixedWidth(80);
  layout->addWidget(addr_end, 1, BreakAddrEnd);

  data = new QLineEdit;
  data->setFixedWidth(40);
  layout->addWidget(data, 1, BreakData);

  mode_r = new QCheckBox;
  layout->addWidget(mode_r, 1, BreakRead);
  mode_w = new QCheckBox;
  layout->addWidget(mode_w, 1, BreakWrite);
  mode_x = new QCheckBox;
  layout->addWidget(mode_x, 1, BreakExecute);

  source = new QComboBox;
  source->addItem("S-CPU bus");
  source->addItem("S-SMP bus");
  source->addItem("S-PPU VRAM");
  source->addItem("S-PPU OAM");
  source->addItem("S-PPU CGRAM");
  source->addItem("SA-1 bus");
  source->addItem("SuperFX bus");
  layout->addWidget(source, 1, BreakSource);

  if (id_ == 0) {
    layout->addWidget(new QLabel("Address Range"), 0, BreakAddrStart, 1, BreakAddrEnd - BreakAddrStart + 1);
    layout->addWidget(new QLabel("Data"), 0, BreakData);
	QLabel *label = new QLabel("R");
	label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, 0, BreakRead);
    label = new QLabel("W");
	label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, 0, BreakWrite);
    label = new QLabel("X");
	label->setAlignment(Qt::AlignHCenter);
    layout->addWidget(label, 0, BreakExecute);
    layout->addWidget(new QLabel("Source"), 0, BreakSource);
  }

  connect(enabled, SIGNAL(stateChanged(int)), this, SLOT(toggle()));
}

void BreakpointItem::toggle() {
  bool state = enabled->isChecked();

  if(state) {
    SNES::debugger.breakpoint[id].enabled = true;
    SNES::debugger.breakpoint[id].addr = hex(addr->text().toUtf8().data()) & 0xffffff;
    SNES::debugger.breakpoint[id].addr_end = hex(addr_end->text().toUtf8().data()) & 0xffffff;
    if(addr_end->text().length() == 0) SNES::debugger.breakpoint[id].addr_end = 0;
    SNES::debugger.breakpoint[id].data = hex(data->text().toUtf8().data()) & 0xff;
    if(data->text().length() == 0) SNES::debugger.breakpoint[id].data = -1;
    
    SNES::debugger.breakpoint[id].mode = 0;
    if(mode_r->isChecked()) SNES::debugger.breakpoint[id].mode |= (unsigned)SNES::Debugger::Breakpoint::Mode::Read;
    if(mode_w->isChecked()) SNES::debugger.breakpoint[id].mode |= (unsigned)SNES::Debugger::Breakpoint::Mode::Write;
    if(mode_x->isChecked()) SNES::debugger.breakpoint[id].mode |= (unsigned)SNES::Debugger::Breakpoint::Mode::Exec;
    
    SNES::debugger.breakpoint[id].source = (SNES::Debugger::Breakpoint::Source)source->currentIndex();
    SNES::debugger.breakpoint[id].counter = 0;

    addr->setEnabled(false);
    addr_end->setEnabled(false);
    data->setEnabled(false);
    mode_r->setEnabled(false);
    mode_w->setEnabled(false);
    mode_x->setEnabled(false);
    source->setEnabled(false);
  } else {
    SNES::debugger.breakpoint[id].enabled = false;

    addr->setEnabled(true);
    addr_end->setEnabled(true);
    data->setEnabled(true);
    mode_r->setEnabled(true);
    mode_w->setEnabled(true);
    mode_x->setEnabled(true);
    source->setEnabled(true);
  }
}

BreakpointEditor::BreakpointEditor() {
  setObjectName("breakpoint-editor");
  setWindowTitle("Breakpoint Editor");
  setGeometryString(&config().geometry.breakpointEditor);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  for(unsigned n = 0; n < SNES::Debugger::Breakpoints; n++) {
    breakpoint[n] = new BreakpointItem(n);
    layout->addWidget(breakpoint[n]);
  }
}
