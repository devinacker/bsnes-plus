#include "image-grid-widget.moc"

const QColor ImageGridWidget::GRID_COLOR = QColor(200, 200, 255, 128);
const QColor ImageGridWidget::SELECTED_INNER_COLOR = QColor(255, 255, 255, 255);
const QColor ImageGridWidget::SELECTED_OUTER_COLOR = QColor(0, 0, 0, 255);

ImageGridWidget::ImageGridWidget() {
  pixmap = QPixmap();

  scene = new QGraphicsScene;

  scenePixmap = new QGraphicsPixmapItem();
  scenePixmap->setTransformationMode(Qt::FastTransformation);
  scene->addItem(scenePixmap);

  this->setScene(scene);

  showGrid = false;
  gridSize = 8;

  setZoom(1);
  selectNone();
}

QPoint ImageGridWidget::selected() const {
  return selectedCell;
}

bool ImageGridWidget::selectionValid() const {
  return pixmap.rect().contains(selectedCell * int(gridSize));
}

void ImageGridWidget::setImage(const QImage& image) {
  pixmap.convertFromImage(image);
  scenePixmap->setPixmap(pixmap);
  scene->setSceneRect(scenePixmap->boundingRect());
}

void ImageGridWidget::setZoom(unsigned z) {
  if(z == 0) z = 1;

  resetTransform();
  scale(z, z);

  scene->setSceneRect(scenePixmap->boundingRect());
}

void ImageGridWidget::setGridSize(unsigned g) {
  if(g == 0) g = 8;

  if(g != gridSize) {
    gridSize = g;

    viewport()->update();
  }
}

void ImageGridWidget::setShowGrid(bool s) {
  if(showGrid != s) {
    showGrid = s;

    viewport()->update();
  }
}

void ImageGridWidget::selectNone() {
  bool previouslyValid = selectionValid();

  selectedCell = QPoint(-1, -1);

  if(previouslyValid) {
    viewport()->update();
    selectedChanged();
  }
}

void ImageGridWidget::setSelected(const QPoint& cell) {
  if(selectedCell != cell) {
    selectedCell = cell;

    viewport()->update();
    selectedChanged();
  }
}

void ImageGridWidget::mousePressEvent(QMouseEvent* event) {
  if(event->button() != Qt::LeftButton) return;

  QPoint p = mapToScene(event->pos()).toPoint();
  QPoint c(p.x() / gridSize, p.y() / gridSize);

  if(p.x() > 0 && p.y() > 0 && pixmap.rect().contains(c)) {
    setSelected(c);
  } else {
    selectNone();
  }
}

void ImageGridWidget::drawForeground(QPainter* painter, const QRectF& rect) {
  drawGrid(painter, rect);
  drawSelectedCell(painter, rect);
}

void ImageGridWidget::drawGrid(QPainter* painter, const QRectF& rect) {
  if(!showGrid) return;

  QRectF sceneRect = scene->sceneRect();

  qreal top = rect.top() - 1;
  if(top < 0) top = 0;

  qreal bottom = rect.bottom() + 1;
  if(bottom > sceneRect.bottom()) bottom = sceneRect.bottom();

  qreal left = rect.left() - 1;
  if(left < 0) left = 0;

  qreal right = rect.right() + 1;
  if(right > sceneRect.right()) right = sceneRect.right();

  qreal yStart = int(top) + (gridSize - (int(top) % gridSize));
  qreal xStart = int(left) + (gridSize - (int(left) % gridSize));

  painter->save();
  painter->setPen(QPen(GRID_COLOR));

  for(qreal y = yStart; y < bottom; y += gridSize) {
    painter->drawLine(left, y, right, y);
  }
  for(qreal x = xStart; x < right; x += gridSize) {
     painter->drawLine(x, top, x, bottom);
  }

  painter->restore();
}

void ImageGridWidget::drawSelectedCell(QPainter* painter, const QRectF&) {
  if(!selectionValid()) return;

  painter->save();

  QRectF cell(selectedCell.x() * gridSize, selectedCell.y() * gridSize,
              gridSize, gridSize);
  cell = painter->combinedTransform().mapRect(cell);

  painter->setBrush(QBrush());

  painter->resetTransform();

  painter->setPen(QPen(SELECTED_INNER_COLOR, 1));
  painter->drawRect(cell);

  cell.adjust(-1, -1, 1, 1);
  painter->setPen(QPen(SELECTED_OUTER_COLOR, 1));
  painter->drawRect(cell);

  painter->restore();
}
