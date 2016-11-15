class DebuggerOptions : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QCheckBox *cacheUsageBox;
  QCheckBox *showHClocksBox;

  void synchronize();
  DebuggerOptions();

public slots:
  void toggleCacheUsage(bool);
  void toggleHClocks(bool);
};

extern DebuggerOptions *debuggerOptions;
