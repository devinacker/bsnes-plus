#include "memory.moc"
MemoryEditor *memoryEditor;

MemoryEditor::MemoryEditor() {
  setObjectName("memory-editor");
  setWindowTitle("Memory Editor");
  setGeometryString(&config().geometry.memoryEditor);
  application.windowList.append(this);

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  editor = new MemHexEditor;
  editor->reader = { &MemoryEditor::reader, this };
  editor->writer = { &MemoryEditor::writer, this };
  editor->usage  = { &MemoryEditor::usage, this };
  editor->setFont(QFont(Style::Monospace));
  editor->setMinimumWidth((editor->lineWidth() + 3) * editor->fontMetrics().width(' '));
  editor->setMinimumHeight((16 + 1) * editor->fontMetrics().height());
  editor->setSize(16 * 1024 * 1024);
  memorySource = SNES::Debugger::MemorySource::CPUBus;
  layout->addWidget(editor);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  source = new QComboBox;
  source->addItem("S-CPU bus");
  source->addItem("S-APU bus");
  source->addItem("S-PPU VRAM");
  source->addItem("S-PPU OAM");
  source->addItem("S-PPU CGRAM");
  source->addItem("Cartridge ROM");
  source->addItem("Cartridge RAM");
  source->addItem("SA-1 bus");
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
  #define tool(widget, icon, text, slot) \
    widget = new QToolButton; \
    toolLayout->addWidget(widget); \
    widget->setAutoRaise(true); \
    widget->setIcon(QIcon(":16x16/mem-" icon ".png")); \
    widget->setToolTip(text); \
    connect(widget, SIGNAL(released()), this, SLOT(slot()))
  tool(prevCodeButton, "prev-code",    "Previous Code",    prevCode);
  tool(nextCodeButton, "next-code",    "Next Code",        nextCode);
  tool(prevDataButton, "prev-data",    "Previous Data",    prevData);
  tool(nextDataButton, "next-data",    "Next Data",        nextData);
  tool(prevUnkButton,  "prev-unknown", "Previous Unknown", prevUnknown);
  tool(nextUnkButton,  "next-unknown", "Next Unknown",     nextUnknown);
  #undef tool
  toolLayout->addStretch();

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  controlLayout->addWidget(spacer);

  exportButton = new QPushButton("Export");
  controlLayout->addWidget(exportButton);

  importButton = new QPushButton("Import");
  controlLayout->addWidget(importButton);

  connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
  connect(addr, SIGNAL(textEdited(const QString&)), this, SLOT(updateOffset()));
  connect(addr, SIGNAL(returnPressed()), this, SLOT(updateOffset()));
  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(exportButton, SIGNAL(released()), this, SLOT(exportMemory()));
  connect(importButton, SIGNAL(released()), this, SLOT(importMemory()));
}

void MemoryEditor::autoUpdate() {
  if(SNES::cartridge.loaded() && autoUpdateBox->isChecked()) editor->refresh();
}

void MemoryEditor::synchronize() {
  if(SNES::cartridge.loaded() == false) {
    editor->setHtml("");
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
}

void MemoryEditor::show() {
  Window::show();
  refresh();
}

void MemoryEditor::sourceChanged(int index) {
  switch(index) { default:
    case 0: memorySource = SNES::Debugger::MemorySource::CPUBus; editor->setSize(16 * 1024 * 1024); break;
    case 1: memorySource = SNES::Debugger::MemorySource::APURAM; editor->setSize(64 * 1024);        break;
    case 2: memorySource = SNES::Debugger::MemorySource::VRAM;   editor->setSize(64 * 1024);        break;
    case 3: memorySource = SNES::Debugger::MemorySource::OAM;    editor->setSize(544);              break;
    case 4: memorySource = SNES::Debugger::MemorySource::CGRAM;  editor->setSize(512);              break;
    case 5: memorySource = SNES::Debugger::MemorySource::CartROM; editor->setSize(SNES::memory::cartrom.size()); break;
    case 6: memorySource = SNES::Debugger::MemorySource::CartRAM; editor->setSize(SNES::memory::cartram.size()); break;
    case 7: memorySource = SNES::Debugger::MemorySource::SA1Bus; editor->setSize(16 * 1024 * 1024); break;
  }

  editor->setOffset(hex(addr->text().toUtf8().data()));
  editor->refresh();
}

void MemoryEditor::refresh() {
  if(SNES::cartridge.loaded() == false) {
    editor->setHtml("");
  } else {
    if (memorySource == SNES::Debugger::MemorySource::CartROM) {
      editor->setSize(SNES::memory::cartrom.size());
    }
  
    editor->refresh();
  }
}

void MemoryEditor::updateOffset() {
  editor->setOffset(hex(addr->text().toUtf8().data()));
  refresh();
}


void MemoryEditor::prevCode() {
  gotoPrevious(MemHexEditor::UsageExec);
}

void MemoryEditor::nextCode() {
  gotoNext(MemHexEditor::UsageExec);
}

void MemoryEditor::prevData() {
  gotoPrevious(MemHexEditor::UsageRead | MemHexEditor::UsageWrite);
}

void MemoryEditor::nextData() {
  gotoNext(MemHexEditor::UsageRead | MemHexEditor::UsageWrite);
}

void MemoryEditor::prevUnknown() {
  gotoPrevious(0);
}

void MemoryEditor::nextUnknown() {
  gotoNext(0);
}

void MemoryEditor::gotoPrevious(int type) {
  int offset = (int)editor->offset();
  bool found = false;
  SNES::uint8 *usage;
  
  if (memorySource == SNES::Debugger::MemorySource::CPUBus) {
    usage = SNES::cpu.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::APURAM) {
    usage = SNES::smp.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::CartROM) {
    usage = SNES::cpu.cart_usage;
  } 
  else if (memorySource == SNES::Debugger::MemorySource::SA1Bus) {
    usage = SNES::sa1.usage;
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
  int offset = (int)editor->offset();
  unsigned size = editor->size();
  bool found = true;
  SNES::uint8 *usage;
  
  if (memorySource == SNES::Debugger::MemorySource::CPUBus) {
    usage = SNES::cpu.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::APURAM) {
    usage = SNES::smp.usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::CartROM) {
    usage = SNES::cpu.cart_usage;
  }
  else if (memorySource == SNES::Debugger::MemorySource::SA1Bus) {
    usage = SNES::sa1.usage;
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

void MemoryEditor::exportMemory() {
  string basename = filepath(nall::basename(cartridge.fileName), config().path.data);

  if(SNES::memory::cartram.size() != 0 && SNES::memory::cartram.size() != ~0) {
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

  if(SNES::memory::cartram.size() != 0 && SNES::memory::cartram.size() != ~0) {
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
  return SNES::debugger.read(memorySource, addr);
}

void MemoryEditor::writer(unsigned addr, uint8_t data) {
  SNES::debugger.write(memorySource, addr, data);
}

uint8_t MemoryEditor::usage(unsigned addr) {
  if (memorySource == SNES::Debugger::MemorySource::CPUBus && addr < 1 << 24) {
    return SNES::cpu.usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::APURAM && addr < 1 << 16) {
    return SNES::smp.usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::CartROM && addr < 1 << 24) {
    return SNES::cpu.cart_usage[addr];
  }
  else if (memorySource == SNES::Debugger::MemorySource::SA1Bus && addr < 1 << 24) {
    return SNES::sa1.usage[addr];
  }
  
  return 0;
}

void MemHexEditor::refresh() {
  string output;
  char temp[256];
  unsigned offset = editorOffset;

  for(unsigned y = 0; y < editorRows; y++) {
    if(offset >= editorSize) break;
    sprintf(temp, "%.4x:%.4x", (offset >> 16) & 0xffff, (offset >> 0) & 0xffff);
    output << "<font color='#404040'>" << temp << "</font>&nbsp;&nbsp;";

    for(unsigned x = 0; x < editorColumns; x++) {
      if(offset >= editorSize) break;
      sprintf(temp, "%.2x", reader ? reader(offset) : 0x00);
      
      string color;
      uint8_t this_usage = usage ? usage(offset) : 0;
      
	  if (this_usage & UsageExec && this_usage & UsageRead) {
	    color = (x & 1) ? "#800080" : "#ff00ff";
      } else if (this_usage & UsageExec) {
        // code: red text
        color = (x & 1) ? "#800000" : "#ff0000";
      } else if (this_usage & UsageRead || this_usage & UsageWrite) {
        // data: blue text
        color = (x & 1) ? "#000080" : "#0000ff";
      } else {
        // unknown: grey text
        color = (x & 1) ? "#404040" : "#808080";
      }
      
      output << "<font color='" << color << "'>" << temp << "</font>";
      if(x != (editorColumns - 1)) output << "&nbsp;";
      
      offset++;
    }

    if(y != (editorRows - 1)) output << "<br>";
  }

  setHtml(output);
}
