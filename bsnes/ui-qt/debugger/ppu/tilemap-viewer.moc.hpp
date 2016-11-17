
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
  void updateInfo();

  TilemapRenderer renderer;

  QHBoxLayout *layout;
  QFormLayout *sidebarLayout;

  QHBoxLayout *bgLayout;
  QRadioButton *bgButton[4];

  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QComboBox *zoomCombo;

  unsigned layer;

  QLabel *screenMode;
  QLabel *bitDepth;
  QLabel *screenSize;
  QLabel *tileSize;
  QLabel *tileAddr;
  QLabel *screenAddr;

  QGraphicsScene *scene;
  QGraphicsPixmapItem *scenePixmap;

  QGraphicsView* view;
};

extern TilemapViewer *tilemapViewer;
