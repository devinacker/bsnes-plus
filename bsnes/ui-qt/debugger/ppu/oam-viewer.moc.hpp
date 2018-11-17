
class OamCanvas : public QFrame {
  Q_OBJECT

public:
  OamCanvas(OamDataModel* dataModel, OamGraphicsScene* graphicsScene, QWidget* parent);
  void paintEvent(QPaintEvent*);

public slots:
  void refresh();
  void setSelected(int);
  void setScale(unsigned);

private:
  OamDataModel *dataModel;
  OamGraphicsScene *graphicsScene;

  int selected;
  unsigned imageSize;

  QColor backgroundColor;
  QPixmap pixmap;
  int pixmapScale;
};

class OamViewer : public Window {
  Q_OBJECT

public:
  OamViewer();
  void autoUpdate();

public slots:
  void show();
  void refresh();

  void onSelectionChanged();

private:
  OamDataModel *dataModel;
  QSortFilterProxyModel* proxyModel;

  OamGraphicsScene *graphicsScene;

  QVBoxLayout *outerLayout;
  QSplitter *splitter;
  QGraphicsView* graphicsView;

  QWidget* bottomWidget;
  QHBoxLayout *layout;
  QTreeView *treeView;
  QVBoxLayout *controlLayout;
  OamCanvas *canvas;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;

  bool inRefreshCall;
};

extern OamViewer *oamViewer;
