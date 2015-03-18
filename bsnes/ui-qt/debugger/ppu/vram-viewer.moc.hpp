struct VramCanvas : public QWidget {
	Q_OBJECT
public:
    QImage *image;
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    VramCanvas();
signals:
	void infoChanged(unsigned);
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
  QCheckBox *autoUpdateBox;
  QPushButton *refreshButton;
  QLabel *vramInfo;
  VramCanvas *canvas;

  void autoUpdate();
  VramViewer();

public slots:
  void show();
  void refresh();
  void setDepth2bpp();
  void setDepth4bpp();
  void setDepth8bpp();
  void setDepthMode7();
  void displayInfo(unsigned);

private:
  unsigned bpp;
  void refresh2bpp(const uint8_t*, uint32_t*);
  void refresh4bpp(const uint8_t*, uint32_t*);
  void refresh8bpp(const uint8_t*, uint32_t*);
  void refreshMode7(const uint8_t*, uint32_t*);
};

extern VramViewer *vramViewer;
