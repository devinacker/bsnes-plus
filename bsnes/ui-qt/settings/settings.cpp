#include "../ui-base.hpp"

#if defined(LAUNCHER)
  #include "profile.cpp"
#endif

#include "video.cpp"
#include "audio.cpp"
#include "input.cpp"
#include "paths.cpp"
#include "advanced.cpp"
#include "bsx.cpp"

#include "settings.moc"
SettingsWindow *settingsWindow;

SettingsWindow::SettingsWindow() {
  setObjectName("settings-window");
  setWindowTitle("Configuration Settings");
  resize(600, 360);
  setGeometryString(&config().geometry.settingsWindow);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(UIStyle::WindowMargin);
  layout->setSpacing(UIStyle::WidgetSpacing);
  setLayout(layout);

#if defined(LAUNCHER)
  profileSettingsWindow  = new ProfileSettingsWindow;
#endif
  videoSettingsWindow    = new VideoSettingsWindow;
  audioSettingsWindow    = new AudioSettingsWindow;
  inputSettingsWindow    = new InputSettingsWindow;
  pathSettingsWindow     = new PathSettingsWindow;
  advancedSettingsWindow = new AdvancedSettingsWindow;
  bsxSettingsWindow      = new BSXSettingsWindow;

#if defined(LAUNCHER)
  profileArea = new QScrollArea;
  profileArea->setWidget(profileSettingsWindow);
  profileArea->setFrameStyle(0);
  profileArea->setWidgetResizable(true);
#endif

  videoArea = new QScrollArea;
  videoArea->setWidget(videoSettingsWindow);
  videoArea->setFrameStyle(0);
  videoArea->setWidgetResizable(true);

  audioArea = new QScrollArea;
  audioArea->setWidget(audioSettingsWindow);
  audioArea->setFrameStyle(0);
  audioArea->setWidgetResizable(true);

  inputArea = new QScrollArea;
  inputArea->setWidget(inputSettingsWindow);
  inputArea->setFrameStyle(0);
  inputArea->setWidgetResizable(true);

  pathArea = new QScrollArea;
  pathArea->setWidget(pathSettingsWindow);
  pathArea->setFrameStyle(0);
  pathArea->setWidgetResizable(true);

  advancedArea = new QScrollArea;
  advancedArea->setWidget(advancedSettingsWindow);
  advancedArea->setFrameStyle(0);
  advancedArea->setWidgetResizable(true);

  bsxArea = new QScrollArea;
  bsxArea->setWidget(bsxSettingsWindow);
  bsxArea->setFrameStyle(0);
  bsxArea->setWidgetResizable(true);

  tab = new QTabWidget;
#if defined(LAUNCHER)
  tab->addTab(profileArea, "Profile");
#endif
  tab->addTab(videoArea, "Video");
  tab->addTab(audioArea, "Audio");
  tab->addTab(inputArea, "Input");
  tab->addTab(pathArea, "Paths");
  tab->addTab(advancedArea, "Advanced");
  tab->addTab(bsxArea, "BS-X");
  layout->addWidget(tab);
}
