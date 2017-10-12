#include "symbolsview.moc"

// ------------------------------------------------------------------------
SymbolsView::SymbolsView(DisasmProcessor *processor) : processor(processor) {
  setObjectName("symbols");
  setWindowTitle("Symbols");
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  QHBoxLayout *topLayout = new QHBoxLayout();
  layout->addLayout(topLayout);

  search = new QLineEdit();
  topLayout->addWidget(search);

  list = new QTreeWidget;
  list->setColumnCount(3);
  list->setHeaderLabels(QStringList() << "Address" << "Name" << "Description");
  list->setColumnWidth(1, list->fontMetrics().width("  123456789  "));
  list->setAllColumnsShowFocus(true);
  list->sortByColumn(0, Qt::AscendingOrder);
  list->setRootIsDecorated(false);
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);
  list->resizeColumnToContents(0);
  layout->addWidget(list);

  resize(400, 500);
  synchronize();

  connect(processor->getSymbols(), SIGNAL(updated()), this, SLOT(synchronize()));
  connect(list, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(bind(QTreeWidgetItem*, int)));
  connect(search, SIGNAL(textChanged(const QString&)), this, SLOT(synchronize()));
}

// ------------------------------------------------------------------------
void SymbolsView::bind(QTreeWidgetItem *item, int value) {
  if (value != 0) {
    return;
  }

  uint32_t address = item->data(0, Qt::UserRole).toUInt();
  bool enable = item->checkState(0);

  int32_t breakpoint = breakpointEditor->indexOfBreakpointExec(address, processor->getBreakpointBusName());
  if (!enable && breakpoint >= 0) {
    breakpointEditor->removeBreakpoint(breakpoint);
  } else if (enable) {
    breakpointEditor->addBreakpoint(nall::hex(address), "x", processor->getBreakpointBusName());
  }
}

// ------------------------------------------------------------------------
void SymbolsView::synchronize() {
  QString filter = search->text();
  SymbolMap *symbols = processor->getSymbols();

  list->clear();
  list->setSortingEnabled(false);

  uint32_t count = symbols->symbols.size();
  for (uint32_t i=0; i<count; i++) {
    const Symbol &sym = symbols->symbols[i];

    if (filter.length()) {
      QStringList list = filter.split(" ");
      QString search = QString((const char*)sym.name);
      bool found = true;

      for (QStringList::iterator it = list.begin(); it != list.end() && found; it++) {
        if (!search.contains(*it, Qt::CaseInsensitive)) {
          found = false;
        }
      }

      if (!found) {
        continue;
      }
    }

    int32_t breakpoint = breakpointEditor->indexOfBreakpointExec(sym.address, processor->getBreakpointBusName());

    auto item = new QTreeWidgetItem(list);
    item->setData(0, Qt::UserRole, QVariant(sym.address));
    item->setCheckState(0, breakpoint >= 0 ? Qt::Checked : Qt::Unchecked);
    item->setText(0, hex<6, '0'>(sym.address));
    item->setText(1, sym.name);
    item->setText(2, "");
  }

  list->resizeColumnToContents(0);
  list->resizeColumnToContents(1);
  list->setSortingEnabled(true);

}

// ------------------------------------------------------------------------
