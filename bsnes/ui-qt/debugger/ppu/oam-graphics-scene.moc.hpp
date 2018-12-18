
class OamGraphicsScene : public QGraphicsScene {
  Q_OBJECT

public:
  static const int N_OBJECTS = 128;
  static const int N_PALETTES = 8;

  static const int IdRole = 0;

  enum BackgroundType {
    TRANSPARENT_BG,
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

  QSet<int> selectedIds_;

  BackgroundType backgroundType;
  bool showScreenOutline;


public:
  OamGraphicsScene(OamDataModel* dataModel, QObject* parent);

  // setSelectedId DOES NOT emit selectedIdsEdited
  void setSelectedIds(const QSet<int>& selectedIds);
  const QSet<int>& selectedIds() const { return selectedIds_; }

  QRgb backgroundColorForObject(int id);
  QPixmap pixmapForObject(int id);

  QImage renderToImage();

  void setBackrgoundType(BackgroundType type);

protected:
  virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

signals:
  // emitted when the user selects an object manually
  void selectedIdsEdited();

public slots:
  void onSelectionChanged();
  void onModelVisibilityChanged();

  void setShowScreenOutline(bool s);

  void refresh();

private:
  void updateSelectedItems();

  void refreshRectItemColors();

  void updateSpritePalette();
  void updateBackgroundColors();

  void resizeImageBuffer(QImage& imageBuffer, const QSize& size);
  void drawObject(QImage& buffer, const OamObject& obj);
};

