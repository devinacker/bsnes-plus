
class OamGraphicsScene : public QGraphicsScene {
  Q_OBJECT

public:
  static const int N_OBJECTS = 128;
  static const int N_PALETTES = 8;

  static const int IdRole = 0;

  enum BackgroundType {
    TRANSPARENT,
    SCREEN_BG,
    PALETTE_0_BG,
    PALETTE_1_BG,
    PALETTE_2_BG,
    PALETTE_3_BG,
    PALETTE_4_BG,
    PALETTE_5_BG,
    PALETTE_6_BG,
    PALETTE_7_BG,
    MAGENTA,
    CYAN,
    WHITE,
    BLACK,
  };

private:
  OamDataModel* dataModel;

  QImage smallImageBuffer;
  QImage largeImageBuffer;

  // NOTE: the first color for every palette is transparent.
  QRgb spritePalette[N_PALETTES * 16];
  QRgb backgroundColors[N_PALETTES];
  QRgb screenBackgroundColor;

  // items 0 - 127 are the main objects (always shown)
  // items 128 - 255 are the Y axis wrapped objects (sometimes shown)
  QList<QGraphicsPixmapItem*> objects;

  QGraphicsRectItem* backgroundRectItem;
  QGraphicsRectItem* screenOutlineRectItem;

  BackgroundType backgroundType;
  bool showScreenOutline;


public:
  OamGraphicsScene(OamDataModel* dataModel, QObject* parent);

  QRgb backgroundColorForObject(int id);
  QPixmap pixmapForObject(int id);

  QImage renderToImage();

  void setBackrgoundType(BackgroundType type);

public slots:
  void setShowScreenOutline(bool s);

  void refresh();

private:
  void refreshRectItemColors();

  void updateSpritePalette();
  void updateBackgroundColors();

  void resizeImageBuffer(QImage& imageBuffer, const QSize& size);
  void drawObject(QImage& buffer, const OamObject& obj);
};

