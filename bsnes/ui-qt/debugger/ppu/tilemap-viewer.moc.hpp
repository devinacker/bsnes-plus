
class TilemapViewer : public Window {
  Q_OBJECT

public:
  TilemapViewer();
  void autoUpdate();

public slots:
  void show();
  void refresh();

  void onZoomChanged(int);

private:
  void updateRendererSettings();
  void updateForm();

private:
  TilemapRenderer renderer;

  QHBoxLayout *layout;
  QFormLayout *sidebarLayout;
  QHBoxLayout *bgLayout;

  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QComboBox *zoomCombo;

  QCheckBox *customScreenMode;
  QCheckBox *customTilemap;

  QSpinBox  *screenMode;
  QToolButton *bgButtons[4];
  QComboBox *bitDepth;
  QComboBox *screenSize;
  QComboBox *tileSize;
  QLineEdit *tileAddr;
  QLineEdit *screenAddr;

  QGraphicsScene *scene;
  QGraphicsPixmapItem *scenePixmap;

  QGraphicsView* view;

  bool inUpdateFormCall;
};

extern TilemapViewer *tilemapViewer;
