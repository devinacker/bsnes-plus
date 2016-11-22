class SettingsWindow : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QTabWidget *tab;
  QScrollArea *profileArea;
  QScrollArea *videoArea;
  QScrollArea *audioArea;
  QScrollArea *inputArea;
  QScrollArea *pathArea;
  QScrollArea *advancedArea;
  QScrollArea *bsxArea;

  SettingsWindow();

public slots:
};

extern SettingsWindow *settingsWindow;
