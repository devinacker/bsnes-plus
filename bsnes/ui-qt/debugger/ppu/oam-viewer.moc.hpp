struct OamObject {
  unsigned width;
  unsigned height;
  signed   xpos;
  unsigned ypos;
  unsigned character;
  unsigned priority;
  unsigned palette;
  bool hFlip;
  bool vFlip;
  bool table;

  static OamObject getObject(unsigned);
};

class OamCanvas : public QFrame {
  Q_OBJECT

public:
  OamCanvas();
  void paintEvent(QPaintEvent*);

public slots:
  void refresh();
  void setSelected(int);
  void setScale(unsigned);

private:
  void refreshImage(const OamObject& obj);
  unsigned maximumOamBaseSize();

private:
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

  void onSelectedChanged();

private:
  QHBoxLayout *layout;
  QTreeWidget *list;
  QVBoxLayout *controlLayout;
  OamCanvas *canvas;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;

  bool inRefreshCall;
};

extern OamViewer *oamViewer;
