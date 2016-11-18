
class TilemapViewer : public Window {
  Q_OBJECT

public:
  TilemapViewer();
  void autoUpdate();

public slots:
  void show();
  void refresh();

  void onZoomChanged(int);
  void onFormChanged();

private:
  TilemapRenderer renderer;

  QHBoxLayout *layout;
  QFormLayout *sidebarLayout;

  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QComboBox *zoomCombo;

  QComboBox *bitDepth;
  QComboBox *screenSize;
  QComboBox *tileSize;
  QLineEdit *tileAddr;
  QLineEdit *screenAddr;

  QGraphicsScene *scene;
  QGraphicsPixmapItem *scenePixmap;

  QGraphicsView* view;
};

extern TilemapViewer *tilemapViewer;
