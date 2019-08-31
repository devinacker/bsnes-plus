#include "paths.moc"
PathSettingsWindow *pathSettingsWindow;

PathSettingWidget::PathSettingWidget(string &pathValue_, const char *labelText, const char *pathDefaultLabel_, const char *pathBrowseLabel_, const char *pathDefaultValue_) : pathValue(pathValue_) {
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
  path->setPlaceholderText(pathDefaultLabel_);
  if (pathValue != pathDefaultValue)
    path->setText(pathValue);
  controlLayout->addWidget(path);

  pathSelect = new QPushButton("Select ...");
  controlLayout->addWidget(pathSelect);

  pathDefault = new QPushButton("Default");
  controlLayout->addWidget(pathDefault);

  connect(path, SIGNAL(textChanged(QString)), this, SLOT(updatePath()));
  connect(pathSelect, SIGNAL(clicked(bool)), this, SLOT(selectPath()));
  connect(pathDefault, SIGNAL(clicked(bool)), path, SLOT(clear()));
}

void PathSettingWidget::acceptPath(const string &newPath) {
  fileBrowser->close();
  path->setText(string() << newPath << "/");
  config().path.current.folder = dir(newPath);
}

void PathSettingWidget::updatePath() {
  if (path->text().isEmpty()) {
    pathValue = pathDefaultValue;
  } else {
    pathValue = path->text();
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
  
  firmwarePath = new PathSettingWidget(SNES::config().path.firmware, "Extra chip firmware:", "Same as loaded game", "Default Chip Firmware Path", "");
  satdataPath  = new PathSettingWidget(SNES::config().path.bsxdat,  "Satellaview signal data:", "./bsxdat/", "Default Satellaview Signal Data Path", "./bsxdat/");

  layout->addWidget(gamePath);
  layout->addWidget(savePath);
  layout->addWidget(statePath);
  layout->addWidget(patchPath);
  layout->addWidget(cheatPath);
  layout->addWidget(dataPath);
  layout->addWidget(firmwarePath);
  layout->addWidget(satdataPath);
}
