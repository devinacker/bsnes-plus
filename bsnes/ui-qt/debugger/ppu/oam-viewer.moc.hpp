
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

  void onExportClicked();

  void onZoomChanged(int index);
  void onBackgroundChanged(int index);

  void onTreeViewSelectionChanged();
  void onGraphicsSceneSelectedIdsEdited();

  void onToggleVisibility();
  void onShowOnlySelectedObjects();

private:
  void updateActions();

private:
  OamDataModel *dataModel;
  QSortFilterProxyModel* proxyModel;

  OamGraphicsScene *graphicsScene;

  QVBoxLayout *outerLayout;
  QSplitter *splitter;
  QGraphicsView* graphicsView;

  QWidget* bottomWidget;
  QHBoxLayout *layout;
  QHBoxLayout *buttonLayout;
  QTreeView *treeView;

  QFormLayout *sidebarLayout;
  OamCanvas *canvas;
  QComboBox *zoomCombo;
  QCheckBox *autoUpdateBox;
  QPushButton *exportButton;
  QPushButton *refreshButton;
  QCheckBox *showScreenOutlineBox;
  QComboBox *backgroundCombo;
  QLineEdit *firstSprite;

  QAction* toggleVisibility;
  QAction* showOnlySelectedObjects;
  QAction* showAllObjects;

  bool inRefreshCall;
};

extern OamViewer *oamViewer;
