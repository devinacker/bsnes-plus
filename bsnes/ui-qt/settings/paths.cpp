#include "paths.moc"
PathSettingsWindow *pathSettingsWindow;

PathSettingWidget::PathSettingWidget(string &pathValue_, const char *labelText, const char *pathDefaultLabel_, const char *pathBrowseLabel_, const char *pathDefaultValue_) : pathValue(pathValue_) {
  pathDefaultLabel = pathDefaultLabel_;
  pathBrowseLabel = pathBrowseLabel_;
  pathDefaultValue = pathDefaultValue_;

  layout = new QVBoxLayout;
  layout->setMargin(0);
  layout->setSpacing(0);
  setLayout(layout);

  label = new QLabel(labelText);
  layout->addWidget(label);

  controlLayout = new QHBoxLayout;
  controlLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(controlLayout);

  path = new QLineEdit;
  path->setReadOnly(true);
  controlLayout->addWidget(path);

  pathSelect = new QPushButton("Select ...");
  controlLayout->addWidget(pathSelect);

  pathDefault = new QPushButton("Default");
  controlLayout->addWidget(pathDefault);

  connect(pathSelect, SIGNAL(released()), this, SLOT(selectPath()));
  connect(pathDefault, SIGNAL(released()), this, SLOT(defaultPath()));
  updatePath();
}

void PathSettingWidget::acceptPath(const string &newPath) {
  fileBrowser->close();
  pathValue = string() << newPath << "/";
  config().path.current.folder = dir(pathValue);
  updatePath();
}

void PathSettingWidget::updatePath() {
  if(pathValue == pathDefaultValue) {
    path->setStyleSheet("color: #808080");
    path->setText(pathDefaultLabel);
  } else {
    path->setStyleSheet("color: #000000");
    path->setText(pathValue);
  }
}

void PathSettingWidget::selectPath() {
  fileBrowser->onChange.reset();
  fileBrowser->onActivate.reset();
  fileBrowser->onAccept = { &PathSettingWidget::acceptPath, this };
  fileBrowser->setWindowTitle(pathBrowseLabel);
  fileBrowser->setPath(config().path.current.folder);
  fileBrowser->chooseFolder();
}

void PathSettingWidget::defaultPath() {
  pathValue = pathDefaultValue;
  updatePath();
}

PathSettingsWindow::PathSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  gamePath  = new PathSettingWidget(config().path.rom,   "Games:",         "Remember last path",  "Default Game Path", "");
  savePath  = new PathSettingWidget(config().path.save,  "Save RAM:",      "Same as loaded game", "Default Save RAM Path", "");
  statePath = new PathSettingWidget(config().path.state, "Save states:",   "Same as loaded game", "Default Save State Path", "");
  patchPath = new PathSettingWidget(config().path.patch, "BPS/UPS/IPS patches:",   "Same as loaded game", "Default BPS/UPS/IPS Patch Path", "");
  cheatPath = new PathSettingWidget(config().path.cheat, "Cheat codes:",   "Same as loaded game", "Default Cheat Code Path", "");
  dataPath  = new PathSettingWidget(config().path.data,  "Exported data:", "Same as loaded game", "Default Exported Data Path", "");
#if defined(PLATFORM_OSX)
  satdataPath  = new PathSettingWidget(SNES::config.sat.path,  "Satellaview signal data:", nall::launchpath() << "bsxdat/", "Default Satellaview Signal Data Path", nall::launchpath() << "bsxdat/");
#else
  satdataPath  = new PathSettingWidget(SNES::config.sat.path,  "Satellaview signal data:", "./bsxdat/", "Default Satellaview Signal Data Path", "./bsxdat/");
#endif

  layout->addWidget(gamePath);
  layout->addWidget(savePath);
  layout->addWidget(statePath);
  layout->addWidget(patchPath);
  layout->addWidget(cheatPath);
  layout->addWidget(dataPath);
  layout->addWidget(satdataPath);
}
