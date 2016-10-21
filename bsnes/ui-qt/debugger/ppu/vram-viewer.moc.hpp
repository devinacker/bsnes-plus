struct VramCanvas : public QWidget {
	Q_OBJECT
public:
  QImage *image;
  void paintEvent(QPaintEvent*);
  void mousePressEvent(QMouseEvent*);
  VramCanvas();

public slots:
  void refresh();
  void setDepth2bpp();
  void setDepth4bpp();
  void setDepth8bpp();
  void setDepthMode7();
  void setZoom(unsigned);
  void updateWidgetSize();

signals:
  void infoChanged(unsigned vram_addr);

private:
  unsigned bpp;
  unsigned zoom;
  void refresh2bpp(const uint8_t*);
  void refresh4bpp(const uint8_t*);
  void refresh8bpp(const uint8_t*);
  void refreshMode7(const uint8_t*);
};

class VramViewer : public Window {
  Q_OBJECT

public:
  QVBoxLayout *layout;
  QHBoxLayout *controlLayout;
  QRadioButton *depth2bpp;
  QRadioButton *depth4bpp;
  QRadioButton *depth8bpp;
  QRadioButton *depthMode7;
  QComboBox *zoom;
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QLabel *vramInfo;
  VramCanvas *canvas;
  QScrollArea *scrollArea;

  void autoUpdate();
  VramViewer();

public slots:
  void show();
  void refresh();
  void zoomChanged(int);
  void displayInfo(unsigned vram_addr);
};

extern VramViewer *vramViewer;
