#include "breakpoint.moc"
BreakpointEditor *breakpointEditor;

BreakpointItem::BreakpointItem(unsigned id_) : id(id_) {
  layout = new QGridLayout;
  layout->setMargin(0);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  addr = new QLineEdit;
  addr->setFixedWidth(80);
  layout->addWidget(addr, 1, BreakAddrStart);
  connect(addr, SIGNAL(textChanged(const QString&)), this, SLOT(init()));
  connect(addr, SIGNAL(textChanged(const QString&)), this, SLOT(toggle()));
  
  layout->addWidget(new QLabel(" - "), 1, BreakAddrDash);

  addr_end = new QLineEdit;
  addr_end->setFixedWidth(80);
  layout->addWidget(addr_end, 1, BreakAddrEnd);
  connect(addr_end, SIGNAL(textChanged(const QString&)), this, SLOT(init()));
  connect(addr_end, SIGNAL(textChanged(const QString&)), this, SLOT(toggle()));
  
  data = new QLineEdit;
  data->setFixedWidth(40);
  layout->addWidget(data, 1, BreakData);
  connect(data, SIGNAL(textChanged(const QString&)), this, SLOT(init()));
  connect(data, SIGNAL(textChanged(const QString&)), this, SLOT(toggle()));
  
  mode_r = new QCheckBox;
  layout->addWidget(mode_r, 1, BreakRead);
  connect(mode_r, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  mode_w = new QCheckBox;
  layout->addWidget(mode_w, 1, BreakWrite);
  connect(mode_w, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  mode_x = new QCheckBox;
  layout->addWidget(mode_x, 1, BreakExecute);
  connect(mode_x, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  
  source = new QComboBox;
  source->addItem("S-CPU bus");
  source->addItem("S-SMP bus");
  source->addItem("S-PPU VRAM");
  source->addItem("S-PPU OAM");
  source->addItem("S-PPU CGRAM");
  source->addItem("SA-1 bus");
  source->addItem("SuperFX bus");
  layout->addWidget(source, 1, BreakSource);
  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(init()));
  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(toggle()));
  
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
  
  init();
}

void BreakpointItem::init() {
  SNES::debugger.breakpoint[id].enabled = false;
  SNES::debugger.breakpoint[id].counter = 0;
}

bool BreakpointItem::isEnabled() const {
  return SNES::debugger.breakpoint[id].enabled;
}

uint32_t BreakpointItem::getAddressFrom() const {
  return SNES::debugger.breakpoint[id].addr;
}

uint32_t BreakpointItem::getAddressTo() const {
  if (SNES::debugger.breakpoint[id].addr_end == 0) {
    return SNES::debugger.breakpoint[id].addr;
  } else {
    return SNES::debugger.breakpoint[id].addr_end;
  }
}

bool BreakpointItem::isModeR() const {
  return SNES::debugger.breakpoint[id].mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Read;
}

bool BreakpointItem::isModeW() const {
  return SNES::debugger.breakpoint[id].mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Write;
}

bool BreakpointItem::isModeX() const {
  return SNES::debugger.breakpoint[id].mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Exec;
}

string BreakpointItem::getBus() const {
  switch (source->currentIndex()) {
  default:
  case 0: return "cpu";
  case 1: return "smp";
  case 2: return "vram";
  case 3: return "oam";
  case 4: return "cgram";
  case 5: return "sa1";
  case 6: return "sfx";
  }

  return "";
}

void BreakpointItem::toggle() {
  bool state = mode_r->isChecked() | mode_w->isChecked() | mode_x->isChecked();
  SNES::debugger.breakpoint[id].enabled = state;
    
  if(state) {
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
  }
}

void BreakpointItem::clear() {
  addr->setText("");
  addr_end->setText("");
  data->setText("");
  
  mode_r->setChecked(false);
  mode_w->setChecked(false);
  mode_x->setChecked(false);
  
  source->setCurrentIndex(0);
}

void BreakpointItem::removeBreakpoint() {
  clear();
  toggle();
}

void BreakpointItem::setBreakpoint(string addrStr, string mode, string sourceStr) {
  if (addrStr == "") return;

  sourceStr.lower();
  if(sourceStr == "cpu")        { source->setCurrentIndex(0); }
  else if(sourceStr == "smp")   { source->setCurrentIndex(1); }
  else if(sourceStr == "vram")  { source->setCurrentIndex(2); }
  else if(sourceStr == "oam")   { source->setCurrentIndex(3); }
  else if(sourceStr == "cgram") { source->setCurrentIndex(4); }
  else if(sourceStr == "sa1")   { source->setCurrentIndex(5); }
  else if(sourceStr == "sfx")   { source->setCurrentIndex(6); }
  else { return; }

  mode.lower();
  mode_r->setChecked(mode.position("r"));
  mode_w->setChecked(mode.position("w"));
  mode_x->setChecked(mode.position("x"));

  lstring addresses;
  addresses.split<2>("=", addrStr);
  if (addresses.size() >= 2) { data->setText(addresses[1]); }
  
  addrStr = addresses[0];
  addresses.split<2>("-", addrStr);
  addr->setText(addresses[0]);
  if (addresses.size() >= 2) { addr_end->setText(addresses[1]); }

  toggle();
}

string BreakpointItem::toString() const {
  if (addr->text().isEmpty()) return "";
  
  string breakpoint;
  
  breakpoint << addr->text().toUtf8().data();
  if (!addr_end->text().isEmpty()) {
    breakpoint << "-" << addr_end->text().toUtf8().data();
  }
  if (!data->text().isEmpty()) {
    breakpoint << "=" << data->text().toUtf8().data();
  }
  
  breakpoint << ":";
  if (mode_r->isChecked()) breakpoint << "r";
  if (mode_w->isChecked()) breakpoint << "w";
  if (mode_x->isChecked()) breakpoint << "x";
  
  breakpoint << ":" << getBus();
  
  return breakpoint;
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

  breakOnWDM = new QCheckBox("Break on WDM (CPU/SA-1 opcode 0x42)");
  breakOnWDM->setChecked(SNES::debugger.break_on_wdm);
  connect(breakOnWDM, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  layout->addWidget(breakOnWDM);

  breakOnBRK = new QCheckBox("Break on BRK (CPU/SA-1 opcode 0x00)");
  breakOnBRK->setChecked(SNES::debugger.break_on_brk);
  connect(breakOnBRK, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  layout->addWidget(breakOnBRK);
}

void BreakpointEditor::toggle() {
  SNES::debugger.break_on_brk = breakOnBRK->isChecked();
  SNES::debugger.break_on_wdm = breakOnWDM->isChecked();
}

void BreakpointEditor::clear() {
  for(unsigned n = 0; n < SNES::Debugger::Breakpoints; n++) {
    breakpoint[n]->clear();
  }
}

void BreakpointEditor::setBreakOnBrk(bool b) {
  breakOnBRK->setChecked(b);
  SNES::debugger.break_on_brk = b;
}

void BreakpointEditor::addBreakpoint(const string& addr, const string& mode, const string& source) {
  for(unsigned n = 0; n < SNES::Debugger::Breakpoints; n++) {
    if(breakpoint[n]->addr->text().isEmpty()) {
      breakpoint[n]->setBreakpoint(addr, mode, source);
      return;
    }
  }
}

void BreakpointEditor::addBreakpoint(const string& breakpoint) {
  lstring param;
  param.split<3>(":", breakpoint);
  if(param.size() == 1) { param.append("rwx"); }
  if(param.size() == 2) { param.append("cpu"); }
  
  this->addBreakpoint(param[0], param[1], param[2]);
}

void BreakpointEditor::removeBreakpoint(uint32_t index) {
  if (index >= SNES::Debugger::Breakpoints) {
    return;
  }

  breakpoint[index]->removeBreakpoint();
}

int32_t BreakpointEditor::indexOfBreakpointExec(uint32_t addr, const string &source) const {
  for(unsigned n = 0; n < SNES::Debugger::Breakpoints; n++) {
    if(breakpoint[n]->isEnabled() && breakpoint[n]->isModeX() && breakpoint[n]->getAddressFrom() <= addr && breakpoint[n]->getAddressTo() >= addr) {
      return n;
    }
  }

  return -1;
}

string BreakpointEditor::toStrings() const {
  string breakpoints;
  
  for(unsigned n = 0; n < SNES::Debugger::Breakpoints; n++) {
    if(!breakpoint[n]->addr->text().isEmpty()) {
      breakpoints << breakpoint[n]->toString() << "\n";
    }
  }
  
  return breakpoints;
}
