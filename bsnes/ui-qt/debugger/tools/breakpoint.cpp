#include "breakpoint.moc"
BreakpointEditor *breakpointEditor;

const QStringList BreakpointModel::sources = {
  "S-CPU bus",
  "S-SMP bus",
  "S-PPU VRAM",
  "S-PPU OAM",
  "S-PPU CGRAM",
  "SA-1 bus",
  "SuperFX bus",
};

const QStringList BreakpointModel::compares = {
  "=",
  "!=",
  "<",
  "<=",
  ">",
  ">="
};

BreakpointModel::BreakpointModel(QObject* parent)
  : QAbstractTableModel(parent) {
}

int BreakpointModel::rowCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : SNES::debugger.breakpoint.size();
}

int BreakpointModel::columnCount(const QModelIndex& parent) const {
  return parent.isValid() ? 0 : BreakColumnCount;
}

QString BreakpointModel::displayAddr(unsigned addr, SNES::Debugger::Breakpoint::Source source) const {
  SymbolMap *symbolMap = 0;
  
  switch (source) {
  case SNES::Debugger::Breakpoint::Source::CPUBus:
    symbolMap = debugger->symbolsCPU;
    break;
  case SNES::Debugger::Breakpoint::Source::APURAM:
    symbolMap = debugger->symbolsSMP;
    break;
  case SNES::Debugger::Breakpoint::Source::SA1Bus:
    symbolMap = debugger->symbolsSA1;
    break;
  }
  
  if (symbolMap) {
    Symbol symbol = symbolMap->getSymbol(addr);
    if (symbol.isSymbol()) return symbol.name;
  }

  return QString::asprintf("%06x", addr);
}

QVariant BreakpointModel::data(const QModelIndex &index, int role) const {
  if (index.row() >= SNES::debugger.breakpoint.size())
    return QVariant();

  const SNES::Debugger::Breakpoint& b = SNES::debugger.breakpoint[index.row()];
    
  if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::ToolTipRole) {
    switch (index.column()) {
    case BreakAddrStart:
      if (role == Qt::DisplayRole) return displayAddr(b.addr, b.source);
      return QString::asprintf("%06x", b.addr);
    case BreakAddrEnd:
      if (b.addr_end) {
        if (role == Qt::DisplayRole) return displayAddr(b.addr_end, b.source);
        return QString::asprintf("%06x", b.addr_end);
      }
      break;
    case BreakCompare:
      if (role == Qt::EditRole) return (unsigned)b.compare;
      if ((unsigned)b.compare < compares.count()) return compares[(unsigned)b.compare];
      break;
    case BreakData:
      if (b.data >= 0) return QString::asprintf("%02x", b.data);
      if (role == Qt::DisplayRole) return "any";
      break;
    case BreakRead:
      return (bool)(b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Read);
    case BreakWrite:
      return (bool)(b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Write);
    case BreakExecute:
      return (bool)(b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Exec);
    case BreakSource:
      if (role == Qt::EditRole) return (unsigned)b.source;
      if ((unsigned)b.source < sources.count()) return sources[(unsigned)b.source];
      break;
    }
    
  }

  return QVariant();
}

QVariant BreakpointModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
      case BreakAddrStart: return "Start";
      case BreakAddrEnd:   return "End (optional)";
      case BreakCompare:   return "Data";
      case BreakRead:      return "R";
      case BreakWrite:     return "W";
      case BreakExecute:   return "X";
      case BreakSource:    return "Source";
      }
    } else {
      return QString::number(section);
    }
  }
  
  return QVariant();
}

bool BreakpointModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role == Qt::EditRole && index.row() < SNES::debugger.breakpoint.size()) {
    SNES::Debugger::Breakpoint& b = SNES::debugger.breakpoint[index.row()];
    
    switch (index.column()) {
    case BreakAddrStart: 
      b.addr = hex(value.toString().toUtf8().data()) & 0xffffff;
      emit dataChanged(index, index);
      return true;
      
    case BreakAddrEnd:
      if (!value.toString().isEmpty())
        b.addr_end = hex(value.toString().toUtf8().data()) & 0xffffff;
      else
        b.addr_end = 0;
      emit dataChanged(index, index);
      return true;
      
    case BreakCompare:
      b.compare = (SNES::Debugger::Breakpoint::Compare)value.toInt();
      emit dataChanged(index, index);
      break;
      
    case BreakData:
      if (!value.toString().isEmpty())
        b.data = hex(value.toString().toUtf8().data()) & 0xff;
      else
        b.data = -1;
      emit dataChanged(index, index);
      return true;
      
    case BreakRead:
      if (value.toBool())
        b.mode |= (unsigned)SNES::Debugger::Breakpoint::Mode::Read;
      else
        b.mode &= ~(unsigned)SNES::Debugger::Breakpoint::Mode::Read;
      emit dataChanged(index, index);
      return true;
      
    case BreakWrite:
      if (value.toBool())
        b.mode |= (unsigned)SNES::Debugger::Breakpoint::Mode::Write;
      else
        b.mode &= ~(unsigned)SNES::Debugger::Breakpoint::Mode::Write;
      emit dataChanged(index, index);
      return true;
      
    case BreakExecute:
      if (value.toBool())
        b.mode |= (unsigned)SNES::Debugger::Breakpoint::Mode::Exec;
      else
        b.mode &= ~(unsigned)SNES::Debugger::Breakpoint::Mode::Exec;
      emit dataChanged(index, index);
      return true;
      
    case BreakSource:
      b.source = (SNES::Debugger::Breakpoint::Source)value.toInt();
      // also refresh start+end addresses (for formatting/labels)
      emit dataChanged(this->index(index.row(), BreakAddrStart), index);
      return true;
    }
  }
  
  return false;
}

Qt::ItemFlags BreakpointModel::flags(const QModelIndex &index) const {
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool BreakpointModel::insertRows(int row, int count, const QModelIndex &parent) {
  if (!parent.isValid() && row <= rowCount() && count > 0) {
    beginInsertRows(parent, row, row + count - 1);
    while (count--)
      SNES::debugger.breakpoint.insert(row, SNES::Debugger::Breakpoint());
    endInsertRows();
    
    return true;
  }

  return false;
}

bool BreakpointModel::removeRows(int row, int count, const QModelIndex &parent) {
  if (!parent.isValid() && row <= rowCount() && count > 0) {
    beginRemoveRows(parent, row, row + count - 1);
    SNES::debugger.breakpoint.remove(row, count);
    endRemoveRows();
    
    return true;
  }

  return false;
}

BreakpointEditor::BreakpointEditor() {
  setObjectName("breakpoint-editor");
  setWindowTitle("Breakpoint Editor");
  setGeometryString(&config().geometry.breakpointEditor);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  model = new BreakpointModel(this);

  table = new QTableView;
  table->setModel(model);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::ContiguousSelection);
  table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
  layout->addWidget(table);
  
  CheckDelegate *checkDelegate = new CheckDelegate(this);
  table->setItemDelegateForColumn(BreakpointModel::BreakRead, checkDelegate);
  table->setItemDelegateForColumn(BreakpointModel::BreakWrite, checkDelegate);
  table->setItemDelegateForColumn(BreakpointModel::BreakExecute, checkDelegate);
  
  table->setItemDelegateForColumn(BreakpointModel::BreakCompare, new ComboDelegate(BreakpointModel::compares, this));
  table->setItemDelegateForColumn(BreakpointModel::BreakSource, new ComboDelegate(BreakpointModel::sources, this));
  
  QHeaderView *header = table->horizontalHeader();
  header->setSectionsClickable(false);
  header->setStretchLastSection(true);
  header->setDefaultAlignment(Qt::AlignLeft);
  header->setSectionResizeMode(BreakpointModel::BreakCompare, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(BreakpointModel::BreakData, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(BreakpointModel::BreakRead, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(BreakpointModel::BreakWrite, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(BreakpointModel::BreakExecute, QHeaderView::ResizeToContents);
  
  header = table->verticalHeader();
  header->setSectionResizeMode(QHeaderView::ResizeToContents);
  
  btnLayout = new QHBoxLayout;
  btnLayout->setMargin(0);
  btnLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(btnLayout);
  
  btnAdd = new QPushButton; 
  btnAdd->setText("Add");
  connect(btnAdd, SIGNAL(clicked(bool)), this, SLOT(add()));
  btnLayout->addWidget(btnAdd);

  btnRemove = new QPushButton;
  btnRemove->setText("Del");
  connect(btnRemove, SIGNAL(clicked(bool)), this, SLOT(remove()));
  btnLayout->addWidget(btnRemove);
  
  btnLayout->addStretch(1);

  breakOnWDM = new QCheckBox("Break on WDM (CPU/SA-1 opcode 0x42)");
  breakOnWDM->setChecked(SNES::debugger.break_on_wdm);
  connect(breakOnWDM, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  layout->addWidget(breakOnWDM);

  breakOnBRK = new QCheckBox("Break on BRK (CPU/SA-1 opcode 0x00)");
  breakOnBRK->setChecked(SNES::debugger.break_on_brk);
  connect(breakOnBRK, SIGNAL(toggled(bool)), this, SLOT(toggle()));
  layout->addWidget(breakOnBRK);
}

void BreakpointEditor::add() {
  model->insertRow(model->rowCount());
}

void BreakpointEditor::remove() {
  QModelIndexList selected = table->selectionModel()->selectedRows();
  qSort(selected);
  if (selected.count() > 0) {
    model->removeRows(selected[0].row(), selected.count());
  }
}

void BreakpointEditor::toggle() {
  SNES::debugger.break_on_brk = breakOnBRK->isChecked();
  SNES::debugger.break_on_wdm = breakOnWDM->isChecked();
}

void BreakpointEditor::clear() {
  model->removeRows(0, model->rowCount());
}

void BreakpointEditor::setBreakOnBrk(bool b) {
  breakOnBRK->setChecked(b);
  SNES::debugger.break_on_brk = b;
}

void BreakpointEditor::addBreakpoint(const string& addr, const string& mode, const string& source) {
  if (addr == "") return;
  
  int row = model->rowCount();
  if (model->insertRow(row)) {
    string sourceStr = source;
    sourceStr.lower();
    int nSource;
    if(sourceStr == "cpu")        { nSource = 0; }
    else if(sourceStr == "smp")   { nSource = 1; }
    else if(sourceStr == "vram")  { nSource = 2; }
    else if(sourceStr == "oam")   { nSource = 3; }
    else if(sourceStr == "cgram") { nSource = 4; }
    else if(sourceStr == "sa1")   { nSource = 5; }
    else if(sourceStr == "sfx")   { nSource = 6; }
    else { return; }
    model->setData(model->index(row, BreakpointModel::BreakSource), nSource);

    string modeStr = mode;
    modeStr.lower();
    model->setData(model->index(row, BreakpointModel::BreakRead), (bool)modeStr.position("r"));
    model->setData(model->index(row, BreakpointModel::BreakWrite), (bool)modeStr.position("w"));
    model->setData(model->index(row, BreakpointModel::BreakExecute), (bool)modeStr.position("x"));

    string addrStr;
    lstring addresses;
    // Try to find a comparison operator.
    // Search in reverse through the available ones so that two-character operators (!=, <=, >=) get found first
    for (int i = BreakpointModel::compares.count() - 1; i >= 0; i--) {
      const char *oper = BreakpointModel::compares[i].toUtf8().constData();
      if (addr.position(oper)) {
        addresses.split<2>(oper, addr);
        model->setData(model->index(row, BreakpointModel::BreakCompare), i);
        break;
      }
    }
    if (addresses.size() >= 2) { 
      model->setData(model->index(row, BreakpointModel::BreakData), (const char*)addresses[1]);
      addrStr = addresses[0];
    } else {
      addrStr = addr;
    }
    
    addresses.split<2>("-", addrStr);
    model->setData(model->index(row, BreakpointModel::BreakAddrStart), (const char*)addresses[0]);
    if (addresses.size() >= 2) { model->setData(model->index(row, BreakpointModel::BreakAddrEnd), (const char*)addresses[1]); }
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
  model->removeRow(index);
}

int32_t BreakpointEditor::indexOfBreakpointExec(uint32_t addr, const string &source) const {
  for(unsigned n = 0; n < SNES::debugger.breakpoint.size(); n++) {
    const SNES::Debugger::Breakpoint &b = SNES::debugger.breakpoint[n];
    if((b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Exec) 
       && addr >= b.addr 
       && addr <= (b.addr_end ? b.addr_end : b.addr)) {
      return n;
    }
  }

  return -1;
}

string BreakpointEditor::toStrings() const {
  string breakpoints;
  
  for(unsigned n = 0; n < SNES::debugger.breakpoint.size(); n++) {
    const SNES::Debugger::Breakpoint &b = SNES::debugger.breakpoint[n];
    
    breakpoints << hex<6>(b.addr);
    
    if (b.addr_end) {
      breakpoints << "-" << hex<6>(b.addr_end);
    }
    
    if (b.data >= 0 && (unsigned)b.compare < BreakpointModel::compares.count()) {
      breakpoints << BreakpointModel::compares[(unsigned)b.compare].toUtf8().constData();
      breakpoints << hex<2>(b.data);
    }
    
    breakpoints << ":";
    if (b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Read) breakpoints << "r";
    if (b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Write) breakpoints << "w";
    if (b.mode & (unsigned)SNES::Debugger::Breakpoint::Mode::Exec) breakpoints << "x";
    
    switch ((unsigned)b.source) {
    default:
    case 0: breakpoints << ":cpu\n"; break;
    case 1: breakpoints << ":smp\n"; break;
    case 2: breakpoints << ":vram\n"; break;
    case 3: breakpoints << ":oam\n"; break;
    case 4: breakpoints << ":cgram\n"; break;
    case 5: breakpoints << ":sa1\n"; break;
    case 6: breakpoints << ":sfx\n"; break;
    }
  }
  
  return breakpoints;
}
