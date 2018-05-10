
class TilemapViewer : public Window {
  Q_OBJECT

public:
  TilemapViewer();
  void autoUpdate();

public slots:
  void show();
  void refresh();

  void onZoomChanged(int);
  void onExportClicked();

private:
  void updateRendererSettings();
  void updateForm();
  void updateTileInfo();
  void updateTileInfoNormal();
  void updateTileInfoMode7();

private:
  TilemapRenderer renderer;

  QHBoxLayout *layout;
  QFormLayout *sidebarLayout;
  QHBoxLayout *buttonLayout;
  QHBoxLayout *bgLayout;

  QCheckBox *autoUpdateBox;

  QPushButton *exportButton;
  QPushButton *refreshButton;

  QComboBox *zoomCombo;
  QCheckBox *showGrid;

  QCheckBox *customScreenMode;
  QCheckBox *customTilemap;

  QSpinBox  *screenMode;
  QToolButton *bgButtons[4];
  QComboBox *bitDepth;
  QComboBox *screenSize;
  QComboBox *tileSize;
  QLineEdit *tileAddr;
  QLineEdit *screenAddr;

  QCheckBox *overrideBackgroundColor;
  QComboBox *customBgColorCombo;

  QLabel *tileInfo;

  ImageGridWidget *imageGridWidget;

  bool inUpdateFormCall;
  bool inExportClickedCall;
};

extern TilemapViewer *tilemapViewer;
