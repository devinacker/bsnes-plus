
class TileViewer : public Window {
  Q_OBJECT

  const static unsigned N_VRAM_BASE_ITEMS = 6;
  const static char* VramBaseText[N_VRAM_BASE_ITEMS];

public:
  TileViewer();
  void autoUpdate();

public slots:
  void show();
  void refresh();
  void updateTileInfo();

  void onZoomChanged(int);
  void onExportClicked();

  void onUseCgramPressed();
  void onVramBaseButtonClicked(int);

private:
  void updateRendererSettings();
  void updateForm();

private:
  TileRenderer renderer;

  QHBoxLayout *layout;
  QFormLayout *sidebarLayout;
  QHBoxLayout *buttonLayout;
  QGridLayout *vramBaseLayout;

  QCheckBox *autoUpdateBox;

  QPushButton *exportButton;
  QPushButton *refreshButton;

  QComboBox *zoomCombo;
  QCheckBox *showGrid;

  QComboBox *source;
  QLineEdit *cpuAddress;

  QComboBox *bitDepth;
  QSpinBox  *widthSpinBox;

  QCheckBox *overrideBackgroundColor;
  QComboBox *customBgColorCombo;

  QCheckBox *useCgram;
  CgramWidget *cgramWidget;

  QButtonGroup *vramBaseButtonGroup;
  QLineEdit *vramBaseAddress[N_VRAM_BASE_ITEMS];
  QToolButton *vramBaseButton[N_VRAM_BASE_ITEMS];

  QLabel *tileInfo;

  ImageGridWidget* imageGridWidget;

  bool inUpdateFormCall;
  bool inExportClickedCall;
};

extern TileViewer *tileViewer;
