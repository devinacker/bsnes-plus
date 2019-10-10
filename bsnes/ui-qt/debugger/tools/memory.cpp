#include "memory.moc"
#include "qhexedit.moc"
#include "commands.moc"

#include "qhexedit2/qhexedit.cpp"
#include "qhexedit2/commands.cpp"

QVector <MemoryEditor*> memoryEditors;

MemoryEditor::MemoryEditor() {
  setObjectName("memory-editor");
  setWindowTitle("Memory Editor");
  setGeometryString(&config().geometry.memoryEditor);
  application.windowList.append(this);

  layout = new QGridLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  editor = new QHexEdit;
  editor->setContextMenuPolicy(Qt::CustomContextMenu);
  editor->reader = { &MemoryEditor::reader, this };
  editor->writer = { &MemoryEditor::writer, this };
  editor->usage  = { &MemoryEditor::usage, this };
  memorySource = SNES::Debugger::MemorySource::CPUBus;
  layout->addWidget(editor, 0, 0);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout, 0, 1);
  layout->setColumnStretch(0, 1);

  source = new QComboBox;
  source->addItem("S-CPU bus");
  source->addItem("S-APU bus");
  source->addItem("S-PPU VRAM");
  source->addItem("S-PPU OAM");
  source->addItem("S-PPU CGRAM");
  source->addItem("Cartridge ROM");
  source->addItem("Cartridge RAM");
  source->addItem("SA-1 bus");
  source->addItem("SuperFX bus");
  controlLayout->addWidget(source);
  controlLayout->addSpacing(2);

  addr = new QLineEdit;
  controlLayout->addWidget(addr);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  toolLayout = new QHBoxLayout;
  controlLayout->addLayout(toolLayout);
  #define tool(widget, icon, text, slot, accel) \
    widget = new QToolButton; \
    toolLayout->addWidget(widget); \
    widget->setAutoRaise(true); \
    widget->setDefaultAction(new QAction(this)); \
    widget->defaultAction()->setIcon(QIcon(":16x16/mem-" icon ".png")); \
    widget->defaultAction()->setToolTip(text); \
    widget->defaultAction()->setShortcut(accel); \
    connect(widget->defaultAction(), SIGNAL(triggered()), this, SLOT(slot()))
  tool(prevCodeButton, "prev-code",    "Previous Code",    prevCode, 0);
  tool(nextCodeButton, "next-code",    "Next Code",        nextCode, 0);
  tool(prevDataButton, "prev-data",    "Previous Data",    prevData, 0);
  tool(nextDataButton, "next-data",    "Next Data",        nextData, 0);
  tool(prevUnkButton,  "prev-unknown", "Previous Unknown", prevUnknown, 0);
  tool(nextUnkButton,  "next-unknown", "Next Unknown",     nextUnknown, 0);
  toolLayout->addStretch();
  
  toolLayout = new QHBoxLayout;
  controlLayout->addLayout(toolLayout);
  tool(findButton,     "find",         "Find in Memory (Ctrl+F)",  search, Qt::Key_F | Qt::CTRL);
  // TODO: other icons for these maybe
  tool(findPrevButton, "prev-unknown", "Find again up (Shift+F3)", searchPrev, Qt::Key_F3 | Qt::SHIFT);
  tool(findNextButton, "next-unknown", "Find again down (F3)",     searchNext, Qt::Key_F3);
  toolLayout->addStretch();
  #undef tool
  
  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  controlLayout->addWidget(spacer);

  exportButton = new QPushButton("Export");
  controlLayout->addWidget(exportButton);

  importButton = new QPushButton("Import");
  controlLayout->addWidget(importButton);

  statusBar = new QLabel;
  layout->addWidget(statusBar, 1, 0, 1, 2);

  connect(editor, SIGNAL(currentAddressChanged(qint64)), this, SLOT(showAddress(qint64)));
  connect(editor, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOffset()));
  connect(addr, SIGNAL(textEdited(const QString&)), this, SLOT(updateOffset()));
  connect(addr, SIGNAL(returnPressed()), this, SLOT(updateOffset()));
  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(exportButton, SIGNAL(released()), this, SLOT(exportMemory()));
  connect(importButton, SIGNAL(released()), this, SLOT(importMemory()));
  
  searchPos = -1;
  breakpointPos = -1;
  
  sourceChanged(0);
}

void MemoryEditor::autoUpdate() {
  if(SNES::cartridge.loaded() && autoUpdateBox->isChecked()) {
    editor->refresh(false);
  }
}

void MemoryEditor::closeEvent(QCloseEvent*) {
  int32_t index = memoryEditors.indexOf(this);

  if (index >= 0) {
    memoryEditors.remove(index);
  }
}

void MemoryEditor::synchronize() {
  if(SNES::cartridge.loaded() == false) {
    source->setEnabled(false);
    addr->setEnabled(false);
    autoUpdateBox->setEnabled(false);
    refreshButton->setEnabled(false);
    exportButton->setEnabled(false);
    importButton->setEnabled(false);
  } else {
    source->setEnabled(true);
    addr->setEnabled(true);
    autoUpdateBox->setEnabled(true);
    refreshButton->setEnabled(true);
    exportButton->setEnabled(true);
    importButton->setEnabled(true);
  }
  
  sourceChanged(source->currentIndex());
}

void MemoryEditor::show() {
  Window::show();

  memoryEditors.push_back(this);

  refresh();
}

void MemoryEditor::sourceChanged(int index) {
  switch(index) { default:
    case 0: memorySource = SNES::Debugger::MemorySource::CPUBus; editor->setEditorSize(16 * 1024 * 1024); break;
    case 1: memorySource = SNES::Debugger::MemorySource::APUBus; editor->setEditorSize(64 * 1024);        break;
    case 2: memorySource = SNES::Debugger::MemorySource::VRAM;   editor->setEditorSize(SNES::memory::vram.size()); break;
    case 3: memorySource = SNES::Debugger::MemorySource::OAM;    editor->setEditorSize(544);              break;
    case 4: memorySource = SNES::Debugger::MemorySource::CGRAM;  editor->setEditorSize(512);              break;
    case 5: memorySource = SNES::Debugger::MemorySource::CartROM; editor->setEditorSize(SNES::memory::cartrom.size()); break;
    case 6: memorySource = SNES::Debugger::MemorySource::CartRAM; editor->setEditorSize(SNES::memory::cartram.size()); break;
    case 7: memorySource = SNES::Debugger::MemorySource::SA1Bus; editor->setEditorSize(16 * 1024 * 1024); break;
    case 8: memorySource = SNES::Debugger::MemorySource::SFXBus; editor->setEditorSize(8 * 1024 * 1024); break;
  }
}

void MemoryEditor::refresh() {
  editor->refresh();
}

void MemoryEditor::updateOffset() {
  int offset = hex(addr->text().toUtf8().data());
  
  editor->verticalScrollBar()->setValue(offset / BYTES_PER_LINE);
  editor->setCursorPosition(2 * offset);
  refresh();
}

void MemoryEditor::showAddress(qint64 address) {
  if (address < 0) {
    statusBar->setText("");
  } else {
    statusBar->setText(QString::asprintf("Address: 0x%06X", address));
  }
}

void MemoryEditor::showContextMenu(const QPoint& pos) {
  QMenu menu;
  
  menu.addAction("Cut", editor, SLOT(cut()), QKeySequence::Cut);
  menu.addAction("Copy", editor, SLOT(copy()), QKeySequence::Copy);
  menu.addAction("Paste", editor, SLOT(paste()), QKeySequence::Paste);
  
  menu.addSeparator();
  
  menu.addAction("Undo", editor, SLOT(undo()), QKeySequence::Undo)
    ->setEnabled(editor->canUndo());
  menu.addAction("Redo", editor, SLOT(redo()), QKeySequence::Redo)
    ->setEnabled(editor->canRedo());
    
  if (memorySource != SNES::Debugger::MemorySource::CartROM
      && memorySource != SNES::Debugger::MemorySource::CartRAM) {
    menu.addSeparator();  
    
    breakpointPos = editor->cursorPosition(pos) / 2;

    QMenu *menuBreakpoint = menu.addMenu(QString::asprintf("Add breakpoint at 0x%06X", breakpointPos));
    menuBreakpoint->addAction("Read", this, SLOT(addBreakpointR()));
    menuBreakpoint->addAction("Write", this, SLOT(addBreakpointW()));
    menuBreakpoint->addAction("Execute", this, SLOT(addBreakpointX()));
    menuBreakpoint->addAction("Read/Write", this, SLOT(addBreakpointRW()));
    menuBreakpoint->addAction("Read/Write/Execute", this, SLOT(addBreakpointRWX()));
  }
  
  menu.exec(editor->mapToGlobal(pos));
}

void MemoryEditor::addBreakpoint(const string& mode) {
  if (breakpointPos < 0) return;
  string source;
  
  switch (memorySource) {
  case SNES::Debugger::MemorySource::CPUBus: source = "cpu"; break;
  case SNES::Debugger::MemorySource::APUBus: source = "smp"; break;
  case SNES::Debugger::MemorySource::VRAM:   source = "vram"; break;
  case SNES::Debugger::MemorySource::OAM:    source = "oam"; break;
  case SNES::Debugger::MemorySource::CGRAM:  source = "cgram"; break;
  case SNES::Debugger::MemorySource::SA1Bus: source = "sa1"; break;
  case SNES::Debugger::MemorySource::SFXBus: source = "sfx"; break;
  default: return; // cart ROM/RAM breakpoints not supported
  }
  
  breakpointEditor->addBreakpoint(hex<6>(breakpointPos), mode, source);
  breakpointEditor->show();
}

void MemoryEditor::addBreakpointR() {
  addBreakpoint("r");
}

void MemoryEditor::addBreakpointW() {
  addBreakpoint("w");
}

void MemoryEditor::addBreakpointX() {
  addBreakpoint("x");
}

void MemoryEditor::addBreakpointRW() {
  addBreakpoint("rw");
}

void MemoryEditor::addBreakpointRWX() {
  addBreakpoint("rwx");
}

void MemoryEditor::prevCode() {
  gotoPrevious(QHexEdit::UsageExec);
}

void MemoryEditor::nextCode() {
  gotoNext(QHexEdit::UsageExec);
}

void MemoryEditor::prevData() {
  gotoPrevious(QHexEdit::UsageRead | QHexEdit::UsageWrite);
}

void MemoryEditor::nextData() {
  gotoNext(QHexEdit::UsageRead | QHexEdit::UsageWrite);
}

void MemoryEditor::prevUnknown() {
  gotoPrevious(0);
}

void MemoryEditor::nextUnknown() {
  gotoNext(0);
}

void MemoryEditor::gotoPrevious(int type) {
  int offset = (int)editor->cursorPosition() / 2;
  bool found = false;
  SNES::uint8 *usage;
  
  if (memorySource == SNES::Debugger::MemorySource::CPUBus) {
    usage = SNES::cpu.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::APUBus) {
    usage = SNES::smp.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::CartROM) {
    usage = SNES::cpu.cart_usage;
  } 
  else if (memorySource == SNES::Debugger::MemorySource::SA1Bus) {
    usage = SNES::sa1.usage;
  } 
  else if (memorySource == SNES::Debugger::MemorySource::SFXBus) {
    usage = SNES::superfx.usage;
  } else return;
  
  while (--offset >= 0) {
    bool foundHere = ((type && usage[offset] & type) || (!type && (usage[offset] & 0xf0) == 0));
    
    if (found && !foundHere) {
      offset++; break;
    } else if (!found && foundHere) {
      found = foundHere;
    }
  }
  
  if (offset < 0 && found) offset = 0;
  
  if (offset >= 0) {
    addr->setText(QString::number(offset, 16));
    updateOffset();
  } else {
    QMessageBox::information(this, "Memory Editor", "Reached beginning of memory.", QMessageBox::Ok);
  }
}

void MemoryEditor::gotoNext(int type) {
  int offset = (int)editor->cursorPosition() / 2;
  unsigned size = editor->editorSize();
  bool found = true;
  SNES::uint8 *usage;
  
  if (memorySource == SNES::Debugger::MemorySource::CPUBus) {
    usage = SNES::cpu.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::APUBus) {
    usage = SNES::smp.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::CartROM) {
    usage = SNES::cpu.cart_usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::SA1Bus) {
    usage = SNES::sa1.usage;
  } 
  else if (memorySource == SNES::Debugger::MemorySource::SFXBus) {
    usage = SNES::superfx.usage;
  } else return;
  
  while (++offset < size) {
    bool foundHere = ((type && usage[offset] & type) || (!type && (usage[offset] & 0xf0) == 0));
    
    if (!found && foundHere) {
      found = true; break;
    } else if (found && !foundHere) {
      found = foundHere;
    }
  }
  
  if (offset < size) {
    addr->setText(QString::number(offset, 16));
    updateOffset();
  } else {
    QMessageBox::information(this, "Memory Editor", "Reached end of memory.", QMessageBox::Ok);
  }
}

void MemoryEditor::search() {
  QDialog dlg(this);
  dlg.setWindowTitle("Search Memory");
  
  QVBoxLayout *vbox = new QVBoxLayout;
  dlg.setLayout(vbox);
  
  vbox->addWidget(new QLabel("Enter a hex string, or \"ASCII text\" (in quotes)"));
  
  QLineEdit *edit = new QLineEdit;
  edit->setFont(QFont(Style::Monospace));
  // TODO: put existing search string in box
  vbox->addWidget(edit);
  
  QGridLayout *grid = new QGridLayout;
  vbox->addLayout(grid);
  
  grid->addWidget(new QLabel("Search:"), 0, 0);
  grid->addWidget(new QLabel("Start from:"), 1, 0);
  QButtonGroup bgrpSearch, bgrpStart;
  
  QRadioButton *searchDown = new QRadioButton("Down");
  bgrpSearch.addButton(searchDown);
  grid->addWidget(searchDown, 0, 1);
  searchDown->setChecked(true);
  QRadioButton *searchUp = new QRadioButton("Up");
  bgrpSearch.addButton(searchUp);
  grid->addWidget(searchUp, 0, 2);
  
  QRadioButton *searchFromEnd = new QRadioButton("From start/end");
  bgrpStart.addButton(searchFromEnd);
  grid->addWidget(searchFromEnd, 1, 1);
  searchFromEnd->setChecked(true);
  QRadioButton *searchFromCur = new QRadioButton("From cursor");
  bgrpStart.addButton(searchFromCur);
  grid->addWidget(searchFromCur, 1, 2);
  
  QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(bbox, SIGNAL(accepted()), &dlg, SLOT(accept()));
  connect(bbox, SIGNAL(rejected()), &dlg, SLOT(reject()));
  grid->addWidget(bbox);
  
  if (dlg.exec()) {
    QString searchText = edit->text().trimmed();
    
    // try quoted text
    if (searchText.startsWith("\"") && searchText.endsWith("\"")) {
      searchStr = searchText.mid(1, searchText.size() - 2).toUtf8();
    } else {
      searchStr = QByteArray::fromHex(edit->text().toUtf8());
    }
    
    int offset = (int)editor->cursorPosition() / 2;
    
    if (searchDown->isChecked()) {
      searchPos = searchFromEnd->isChecked() ? -1 : offset;
      searchNext();
    } else {
      searchPos = searchFromEnd->isChecked() ? editor->editorSize() : offset;
      searchPrev();
    }
  }
}

void MemoryEditor::searchNext() {
  if (searchStr.size() == 0) return;

  if (searchPos >= editor->editorSize())
    searchPos = (int)editor->cursorPosition() / 2;
  searchPos = editor->indexOf(searchStr, searchPos + 1);

  if (searchPos >= 0) {
    addr->setText(QString::number(searchPos, 16));
    updateOffset();
  } else {
    QMessageBox::information(this, "Memory Editor", "Reached end of memory.", QMessageBox::Ok);
    searchPos = -1;
  }
}

void MemoryEditor::searchPrev() {
  if (searchStr.size() == 0) return;

  if (searchPos < 0)
    searchPos = (int)editor->cursorPosition() / 2;
  searchPos = editor->lastIndexOf(searchStr, searchPos - 1);

  if (searchPos >= 0) {
    addr->setText(QString::number(searchPos, 16));
    updateOffset();
  } else {
    QMessageBox::information(this, "Memory Editor", "Reached beginning of memory.", QMessageBox::Ok);
    searchPos = editor->editorSize();
  }
}

// TODO: export/import expansion chip memory and cartridge RAM/ROM
void MemoryEditor::exportMemory() {
  string basename = filepath(nall::basename(cartridge.fileName), config().path.data);

  if(SNES::memory::cartram.size() != 0) {
    exportMemory(SNES::memory::cartram, string() << basename << "-sram.bin");
  }
  exportMemory(SNES::memory::wram, string() << basename << "-wram.bin");
  exportMemory(SNES::memory::apuram, string() << basename << "-apuram.bin");
  exportMemory(SNES::memory::vram, string() << basename << "-vram.bin");
  exportMemory(SNES::memory::oam, string() << basename << "-oam.bin");
  exportMemory(SNES::memory::cgram, string() << basename << "-cgram.bin");
}

void MemoryEditor::importMemory() {
  string basename = filepath(nall::basename(cartridge.fileName), config().path.data);

  if(SNES::memory::cartram.size() != 0) {
    importMemory(SNES::memory::cartram, string() << basename << "-sram.bin");
  }
  importMemory(SNES::memory::wram, string() << basename << "-wram.bin");
  importMemory(SNES::memory::apuram, string() << basename << "-apuram.bin");
  importMemory(SNES::memory::vram, string() << basename << "-vram.bin");
  importMemory(SNES::memory::oam, string() << basename << "-oam.bin");
  importMemory(SNES::memory::cgram, string() << basename << "-cgram.bin");
  refresh();  //in case import changed values that are currently being displayed ...
}

void MemoryEditor::exportMemory(SNES::Memory &memory, const string &filename) const {
  file fp;
  if(fp.open(filename, file::mode::write)) {
    for(unsigned i = 0; i < memory.size(); i++) fp.write(memory.read(i));
    fp.close();
  }
}

void MemoryEditor::importMemory(SNES::Memory &memory, const string &filename) const {
  file fp;
  if(fp.open(filename, file::mode::read)) {
    unsigned filesize = fp.size();
    for(unsigned i = 0; i < memory.size() && i < filesize; i++) memory.write(i, fp.read());
    fp.close();
  }
}

uint8_t MemoryEditor::reader(unsigned addr) {
  if (SNES::cartridge.loaded()) {
    SNES::debugger.bus_access = true;
    uint8_t data = SNES::debugger.read(memorySource, addr);
    SNES::debugger.bus_access = false;
    return data;
  }
  
  return 0;
}

void MemoryEditor::writer(unsigned addr, uint8_t data) {
  if (SNES::cartridge.loaded()) {
    SNES::debugger.bus_access = true;
    SNES::debugger.write(memorySource, addr, data);
    SNES::debugger.bus_access = false;
  }
}

uint8_t MemoryEditor::usage(unsigned addr) {
  if (memorySource == SNES::Debugger::MemorySource::CPUBus && addr < 1 << 24) {
    return SNES::cpu.usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::APUBus && addr < 1 << 16) {
    return SNES::smp.usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::CartROM && addr < 1 << 24) {
    return SNES::cpu.cart_usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::SA1Bus && addr < 1 << 24) {
    return SNES::sa1.usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::SFXBus && addr < 1 << 23) {
    return SNES::superfx.usage[addr];
  }
  
  return 0;
}
