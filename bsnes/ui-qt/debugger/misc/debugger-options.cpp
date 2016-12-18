#include "debugger-options.moc"
DebuggerOptions *debuggerOptions;

DebuggerOptions::DebuggerOptions() {
  setObjectName("debugger-options");
  setWindowTitle("Debugger Options");
  setGeometryString(&config().geometry.debuggerOptions);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  cacheUsageBox = new QCheckBox("Cache memory usage table to disk");
  layout->addWidget(cacheUsageBox);
  showHClocksBox = new QCheckBox("Show H-position in clocks instead of dots");
  layout->addWidget(showHClocksBox);

  synchronize();
  connect(cacheUsageBox, SIGNAL(toggled(bool)), this, SLOT(toggleCacheUsage(bool)));
  connect(showHClocksBox, SIGNAL(toggled(bool)), this, SLOT(toggleHClocks(bool)));
}

void DebuggerOptions::synchronize() {
  cacheUsageBox->setChecked(config().debugger.cacheUsageToDisk);
  showHClocksBox->setChecked(config().debugger.showHClocks);
}

void DebuggerOptions::toggleCacheUsage(bool on) {
  config().debugger.cacheUsageToDisk = on;
}

void DebuggerOptions::toggleHClocks(bool on) {
  config().debugger.showHClocks = on;
}
