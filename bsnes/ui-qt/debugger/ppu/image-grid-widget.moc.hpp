
class ImageGridWidget : public QGraphicsView {
  Q_OBJECT

  const static QColor GRID_COLOR;

public:
  ImageGridWidget();

  void setImage(const QImage& image);
  void setZoom(unsigned zoom);
  void setGridSize(unsigned gridSize);

public slots:
  void setShowGrid(bool showGrid);

protected:
  void drawForeground(QPainter* painter, const QRectF& rect);

private:
  bool showGrid;
  unsigned gridSize;

  QPixmap pixmap;

  QGraphicsScene *scene;
  QGraphicsPixmapItem *scenePixmap;
};

