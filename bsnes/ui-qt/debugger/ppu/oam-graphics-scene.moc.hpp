
class OamGraphicsScene : public QGraphicsScene {
  Q_OBJECT

public:
  static const int N_OBJECTS = 128;
  static const int N_PALETTES = 8;

  static const int IdRole = 0;

private:
  OamDataModel* dataModel;

  QImage smallImageBuffer;
  QImage largeImageBuffer;

  // NOTE: the first color for every palette is transparent.
  QRgb spritePalette[N_PALETTES * 16];
  QRgb backgroundColors[N_PALETTES];

  QList<QGraphicsPixmapItem*> objects;

public:
  OamGraphicsScene(OamDataModel* dataModel, QObject* parent);

  QRgb backgroundColorForObject(int id);
  QPixmap pixmapForObject(int id);

  void refresh();

private:
  void updateSpritePalette();
  void updateBackgroundColors();
  void resizeImageBuffer(QImage& imageBuffer, const QSize& size);

  void drawObject(QImage& buffer, const OamObject& obj);
};

