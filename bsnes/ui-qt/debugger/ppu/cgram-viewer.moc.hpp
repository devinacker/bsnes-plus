class CgramViewer : public Window {
  Q_OBJECT

public:
  CgramViewer();

  void autoUpdate();

public slots:
  void show();
  void refresh();

private:
  QHBoxLayout *layout;
  QVBoxLayout *controlLayout;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;

  CgramWidget *cgramWidget;
  QLabel *colorInfo;
};

extern CgramViewer *cgramViewer;
