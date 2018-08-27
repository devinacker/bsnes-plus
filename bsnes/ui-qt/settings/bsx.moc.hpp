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

  QHBoxLayout *sizeLayout;
  QLabel *sizeLabel;
  QComboBox *sizeCombo;

  void initializeUi();
  BSXSettingsWindow();

public slots:
  void timeSettingToggled();
  void customTimeSet();
  void defaultSizeSet();
};

extern BSXSettingsWindow *bsxSettingsWindow;
