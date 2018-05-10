
class ImageGridWidget : public QGraphicsView {
  Q_OBJECT

  const static QColor GRID_COLOR;
  const static QColor SELECTED_INNER_COLOR;
  const static QColor SELECTED_OUTER_COLOR;

public:
  ImageGridWidget();

  void setImage(const QImage& image);
  void setZoom(unsigned zoom);
  void setGridSize(unsigned gridSize);

  QPoint selected() const;
  bool selectionValid() const;

public slots:
  void setShowGrid(bool showGrid);

  void selectNone();
  void setSelected(const QPoint& cell);

  void scrollToCell(const QPoint& cell);

signals:
  void selectedChanged();

protected:
  void mousePressEvent(QMouseEvent *event);

  void drawForeground(QPainter* painter, const QRectF& rect);

private:
  void drawGrid(QPainter* painter, const QRectF& rect);
  void drawSelectedCell(QPainter* painter, const QRectF& rect);

  QPen cosmeticPen(const QColor& color) const;

private:
  bool showGrid;
  unsigned zoom;
  unsigned gridSize;

  QPoint selectedCell;

  QPixmap pixmap;

  QGraphicsScene *scene;
  QGraphicsPixmapItem *scenePixmap;
};

