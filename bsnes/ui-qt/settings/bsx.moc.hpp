class BSXSettingsWindow : public QWidget {
  Q_OBJECT

public:
  QVBoxLayout *layout;

  QHBoxLayout *timeLayout;
  QLabel *timeLabel;
  QRadioButton *useLocalTime;
  QRadioButton *useCustomTime;
  QDateTimeEdit *dateTime;
  QLabel *timeNoteLabel;

  void initializeUi();
  BSXSettingsWindow();

public slots:
  void timeSettingToggled();
  void customTimeSet();
};

extern BSXSettingsWindow *bsxSettingsWindow;
