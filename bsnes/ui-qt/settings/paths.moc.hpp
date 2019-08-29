class PathSettingWidget : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QLabel *label;
  QHBoxLayout *controlLayout;
  QLineEdit *path;
  QPushButton *pathSelect;
  QPushButton *pathDefault;

  string &pathValue;
  string pathBrowseLabel;
  string pathDefaultValue;
  void acceptPath(const string&);

  PathSettingWidget(string&, const char*, const char*, const char*, const char*);

public slots:
  void updatePath();
  void selectPath();
};

class PathSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  PathSettingWidget *gamePath;
  PathSettingWidget *savePath;
  PathSettingWidget *statePath;
  PathSettingWidget *patchPath;
  PathSettingWidget *cheatPath;
  PathSettingWidget *dataPath;
  PathSettingWidget *firmwarePath;
  PathSettingWidget *satdataPath;

  PathSettingsWindow();
};

extern PathSettingsWindow *pathSettingsWindow;
