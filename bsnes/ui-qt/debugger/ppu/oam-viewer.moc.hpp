
class OamCanvas : public QFrame {
  Q_OBJECT

public:
  OamCanvas(OamDataModel* dataModel, QWidget* parent);
  void paintEvent(QPaintEvent*);

public slots:
  void refresh();
  void setSelected(int);
  void setScale(unsigned);

private:
  void refreshImage(const OamObject& obj);
  unsigned maximumOamBaseSize();

private:
  OamDataModel *dataModel;
  int selected;
  unsigned imageSize;
  QImage image;
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

  QHBoxLayout *layout;
  QTreeView *treeView;
  QVBoxLayout *controlLayout;
  OamCanvas *canvas;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;

  bool inRefreshCall;
};

extern OamViewer *oamViewer;
