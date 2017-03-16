#include "image-grid-widget.moc"

const QColor ImageGridWidget::GRID_COLOR = QColor(200, 200, 255, 128);

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

void ImageGridWidget::drawForeground(QPainter* painter, const QRectF& rect) {
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

