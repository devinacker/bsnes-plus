#include "properties.moc"
PropertiesWidget *cpuPropertiesTab;
PropertiesWidget *smpPropertiesTab;
PropertiesWidget *ppuPropertiesTab;
PropertiesWidget *dspPropertiesTab;
PropertiesWidget *sa1PropertiesTab;
PropertiesWidget *sfxPropertiesTab;
PropertiesViewer *propertiesViewer;

void PropertiesWidget::refresh() {
  QList<QTreeWidgetItem*> items = list->findItems("", Qt::MatchContains);
  for(unsigned v = 0; v < items.count(); v++) {
    QTreeWidgetItem *item = items[v];
    unsigned id = item->data(0, Qt::UserRole).toUInt();
    string name, value;
    object.property(id, name, value);
    item->setText(1, value);
  }
}

PropertiesWidget::PropertiesWidget(SNES::ChipDebugger &object) : object(object) {
  setMinimumSize(480, 240);

  layout = new QVBoxLayout;
  setLayout(layout);

  list = new QTreeWidget;
  list->setColumnCount(2);
  list->setHeaderLabels(QStringList() << "Name" << "Value");
  list->setAllColumnsShowFocus(true);
  list->setAlternatingRowColors(true);
  list->setRootIsDecorated(false);
  list->setSortingEnabled(false);
  layout->addWidget(list);

  bool indent = false;
  unsigned counter = 0;
  while(true) {
    string name, value;
    bool result = object.property(counter, name, value);
    if(result == false) break;

    if(value == "") indent = true;
    if(value != "" && indent) name = string("     ", name);

    QTreeWidgetItem *item = new QTreeWidgetItem(list);
    item->setData(0, Qt::UserRole, QVariant(counter++));
    item->setText(0, name);
  }
  for(unsigned i = 0; i <= 1; i++) list->resizeColumnToContents(i);
}

void PropertiesViewer::refresh() {
  cpuPropertiesTab->refresh();
  smpPropertiesTab->refresh();
  ppuPropertiesTab->refresh();
  dspPropertiesTab->refresh();
  sa1PropertiesTab->refresh();
  sfxPropertiesTab->refresh();
}

void PropertiesViewer::show() {
  Window::show();
  refresh();
}

void PropertiesViewer::autoUpdate() {
  if(isVisible() && autoUpdateBox->isChecked()) refresh();
}

PropertiesViewer::PropertiesViewer() {
  setObjectName("properties-viewer");
  setWindowTitle("Properties");
  setGeometryString(&config().geometry.propertiesViewer);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  tabWidget = new QTabWidget;
  layout->addWidget(tabWidget);

  cpuPropertiesTab = new PropertiesWidget(SNES::cpu);
  tabWidget->addTab(cpuPropertiesTab, "S-CPU");

  smpPropertiesTab = new PropertiesWidget(SNES::smp);
  tabWidget->addTab(smpPropertiesTab, "S-SMP");

  ppuPropertiesTab = new PropertiesWidget(SNES::ppu);
  tabWidget->addTab(ppuPropertiesTab, "S-PPU");

  dspPropertiesTab = new PropertiesWidget(SNES::dsp);
  tabWidget->addTab(dspPropertiesTab, "S-DSP");

  sa1PropertiesTab = new PropertiesWidget(SNES::sa1);
  tabWidget->addTab(sa1PropertiesTab, "SA-1");

  sfxPropertiesTab = new PropertiesWidget(SNES::superfx);
  tabWidget->addTab(sfxPropertiesTab, "Super FX");

  controlLayout = new QHBoxLayout;
  controlLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(controlLayout);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
}
