#include "bsx.moc"
BSXSettingsWindow *bsxSettingsWindow;

BSXSettingsWindow::BSXSettingsWindow() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(0);
  layout->setAlignment(Qt::AlignTop);
  setLayout(layout);

  timeLabel = new QLabel("Get BS-X date and time from:");
  layout->addWidget(timeLabel);

  useLocalTime = new QRadioButton("Local time");
  layout->addWidget(useLocalTime);
  
  timeLayout = new QHBoxLayout;
  timeLayout->setSpacing(Style::WidgetSpacing);
  layout->addLayout(timeLayout);
  layout->addSpacing(Style::WidgetSpacing);
  
  useCustomTime = new QRadioButton("Custom time:");
  timeLayout->addWidget(useCustomTime);
  
  dateTime = new QDateTimeEdit;
  timeLayout->addWidget(dateTime);
  
  timeLayout->addStretch();
  
  timeNoteLabel = new QLabel("Note: A system reset is required for a time change to take effect.");
  layout->addWidget(timeNoteLabel);
  
  initializeUi();

  connect(useLocalTime,  SIGNAL(toggled(bool)), this, SLOT(timeSettingToggled()));
  connect(useCustomTime, SIGNAL(toggled(bool)), this, SLOT(timeSettingToggled()));
  connect(dateTime,      SIGNAL(dateTimeChanged(QDateTime)), this, SLOT(customTimeSet()));
}

void BSXSettingsWindow::initializeUi() {
  if (SNES::config.sat.local_time) {
    useLocalTime->setChecked(true);
    dateTime->setEnabled(false);
  } else {
    useCustomTime->setChecked(true);
  }
  
  dateTime->setCalendarPopup(true);
  dateTime->setTimeSpec(Qt::UTC);
  dateTime->setDateTime(QDateTime::fromTime_t(SNES::config.sat.custom_time).toTimeSpec(Qt::UTC));
}

void BSXSettingsWindow::timeSettingToggled() {
  SNES::config.sat.local_time = useLocalTime->isChecked();
  
  dateTime->setEnabled(!SNES::config.sat.local_time);
}

void BSXSettingsWindow::customTimeSet() {
  SNES::config.sat.custom_time = dateTime->dateTime().toTime_t();
}
