#include "oam-graphics-scene.moc"

OamGraphicsScene::OamGraphicsScene(OamDataModel* dataModel, QObject* parent)
  : QGraphicsScene(parent)
  , dataModel(dataModel)
  , smallImageBuffer()
  , largeImageBuffer()
  , spritePalette()
  , objects()
  , backgroundType(BackgroundType::TRANSPARENT_BG)
  , showScreenOutline(true)
{
  const QString toolTipStr = QString::fromLatin1("Sprite %1");

  objects.reserve(N_OBJECTS * 2);

  for(int i = 0; i < N_OBJECTS * 2; i++) {
    int objectId = i % N_OBJECTS;

    QGraphicsPixmapItem* item = new QGraphicsPixmapItem;

    item->setData(IdRole, objectId);
    item->setToolTip(toolTipStr.arg(objectId));

    item->setFlag(QGraphicsItem::ItemIsSelectable, true);
    item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);

    this->addItem(item);

    objects.append(item);
  }

  backgroundRectItem = new QGraphicsRectItem;
  backgroundRectItem->setPen(Qt::NoPen);
  backgroundRectItem->setZValue(-20);
  this->addItem(backgroundRectItem);

  screenOutlineRectItem = new QGraphicsRectItem;
  screenOutlineRectItem->setBrush(Qt::NoBrush);
  screenOutlineRectItem->setZValue(-10);
  this->addItem(screenOutlineRectItem);

  updateBackgroundColors();
  refreshRectItemColors();

  connect(this, SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()));
  connect(dataModel, SIGNAL(visibilityChanged()), this, SLOT(onModelVisibilityChanged()));
}

void OamGraphicsScene::onSelectionChanged() {
  // Don't emit selectionChanged inside this call
  bool oldState = this->blockSignals(true);

  QList<QGraphicsItem*> selected = selectedItems();

  selectedIds_.clear();
  selectedIds_.reserve(selected.size() + 1);

  for(const QGraphicsItem* item : selected) {
    int id = item->data(IdRole).toInt();
    if(id >= 0 && id < N_OBJECTS) {
      selectedIds_.insert(id);

      objects.at(id)->setSelected(true);
      objects.at(id + N_OBJECTS)->setSelected(true);
    }
  }

  this->blockSignals(oldState);

  emit selectedIdsEdited();
}

void OamGraphicsScene::setSelectedIds(const QSet<int>& selectedIds) {
  selectedIds_ = selectedIds;

  updateSelectedItems();
}

void OamGraphicsScene::updateSelectedItems() {
  // Don't emit selectionChanged inside this call
  bool oldState = this->blockSignals(true);

  clearSelection();

  for(const int id : selectedIds_) {
    if(id >= 0 && id < N_OBJECTS) {
      objects.at(id)->setSelected(true);
      objects.at(id + N_OBJECTS)->setSelected(true);
    }
  }

  this->blockSignals(oldState);
}

void OamGraphicsScene::onModelVisibilityChanged() {
  // Don't emit selectionChanged inside this call
  bool oldState = this->blockSignals(true);

  for(int id = 0; id < N_OBJECTS; id++) {
    const bool visible = dataModel->oamObject(id).visible;
    QGraphicsPixmapItem* item = objects.at(id);
    QGraphicsPixmapItem* yWrapedItem = objects.at(N_OBJECTS + id);

    item->setVisible(visible);
    yWrapedItem->setVisible(visible & yWrapedItem->pos().y() < 0);
  }

  updateSelectedItems();

  this->blockSignals(oldState);
}

QImage OamGraphicsScene::renderToImage() {
  // Don't emit selectionChanged inside this call
  bool oldState = this->blockSignals(true);

  clearSelection();

  QImage::Format format = backgroundType == BackgroundType::TRANSPARENT_BG ? QImage::Format_ARGB32 : QImage::Format_RGB32;

  QImage image(sceneRect().size().toSize(), format);
  image.fill(0);

  QPainter painter(&image);
  render(&painter);

  updateSelectedItems();

  this->blockSignals(oldState);

  return image;
}

void OamGraphicsScene::setBackrgoundType(BackgroundType type) {
  if(backgroundType != type) {
    backgroundType = type;
    refreshRectItemColors();
  }
}

void OamGraphicsScene::setShowScreenOutline(bool s) {
  screenOutlineRectItem->setVisible(s);
}

QRgb OamGraphicsScene::backgroundColorForObject(int id) {
  if(id < 0 || id >= N_OBJECTS) return qRgba(0, 0, 0, 0);

  const OamObject& obj = dataModel->oamObject(id);
  return backgroundColors[obj.palette % N_PALETTES];
}

QPixmap OamGraphicsScene::pixmapForObject(int id) {
  if(id < 0 || id >= N_OBJECTS) return QPixmap();

  return objects.at(id)->pixmap();
}

void OamGraphicsScene::refresh() {
  updateSpritePalette();
  updateBackgroundColors();

  const OamDataModel::ObjectSizes& objectSizes = dataModel->objectSizes();
  const unsigned firstSprite = dataModel->firstSprite();

  resizeImageBuffer(smallImageBuffer, objectSizes.small);
  resizeImageBuffer(largeImageBuffer, objectSizes.large);

  for(int id = 0; id < N_OBJECTS; id++) {
    QGraphicsPixmapItem* item = objects.at(id);
    const OamObject& obj = dataModel->oamObject(id);

    int zValue = N_OBJECTS - ((id + N_OBJECTS - firstSprite) % N_OBJECTS) + 100;

    item->setPos(obj.xpos, obj.ypos);
    item->setZValue(zValue);

    if(obj.size == false) {
      drawObject(smallImageBuffer, obj);
      item->setPixmap(QPixmap::fromImage(smallImageBuffer));
    }
    else {
      drawObject(largeImageBuffer, obj);
      item->setPixmap(QPixmap::fromImage(largeImageBuffer));
    }

    QGraphicsPixmapItem* yWrapedItem = objects.at(N_OBJECTS + id);
    const QSize objSize = dataModel->sizeOfObject(obj);
    if(obj.ypos + objSize.height() > 256) {
        yWrapedItem->setPixmap(item->pixmap());
        yWrapedItem->setPos(obj.xpos, int(obj.ypos) - 256);
        yWrapedItem->setZValue(zValue);
        yWrapedItem->setVisible(obj.visible);
        yWrapedItem->setSelected(selectedIds_.contains(id));
    }
    else {
        yWrapedItem->setPos(8192, 8192);
        yWrapedItem->setPixmap(QPixmap());
        yWrapedItem->setVisible(false);
    }
  }

  const int lHeight = objectSizes.large.height();
  const int lWidth = objectSizes.large.width();
  QRectF bRect(-256, -lHeight + 1, 256 * 2 + lWidth - 1, 256 + lHeight * 2 - 2);
  this->setSceneRect(bRect);
  backgroundRectItem->setRect(bRect);

  const int screenHeight = SNES::ppu.overscan() == false ? 224 : 239;
  screenOutlineRectItem->setRect(0, 0, 256, screenHeight);

  refreshRectItemColors();
}

void OamGraphicsScene::refreshRectItemColors()
{
  if(backgroundType == BackgroundType::TRANSPARENT_BG) {
    backgroundRectItem->setBrush(Qt::NoBrush);
    screenOutlineRectItem->setPen(QPen(Qt::black, 0));
    return;
  }

  QRgb bg = qRgb(0, 0, 0);

  switch(backgroundType) {
    case BackgroundType::SCREEN_BG:     bg = screenBackgroundColor; break;
    case BackgroundType::PALETTE_0_BG:  bg = backgroundColors[0];   break;
    case BackgroundType::PALETTE_1_BG:  bg = backgroundColors[1];   break;
    case BackgroundType::PALETTE_2_BG:  bg = backgroundColors[2];   break;
    case BackgroundType::PALETTE_3_BG:  bg = backgroundColors[3];   break;
    case BackgroundType::PALETTE_4_BG:  bg = backgroundColors[4];   break;
    case BackgroundType::PALETTE_5_BG:  bg = backgroundColors[5];   break;
    case BackgroundType::PALETTE_6_BG:  bg = backgroundColors[6];   break;
    case BackgroundType::PALETTE_7_BG:  bg = backgroundColors[7];   break;
    case BackgroundType::MAGENTA:       bg = qRgb(255, 0, 255);     break;
    case BackgroundType::CYAN:          bg = qRgb(0, 255, 255);     break;
    case BackgroundType::WHITE:         bg = qRgb(255, 255, 255);   break;
    case BackgroundType::BLACK:         bg = qRgb(0, 0, 0);         break;
  }
  QColor bgColor(bg);

  backgroundRectItem->setBrush(bgColor);

  QColor screenRectPenColor = bgColor.lightness() > 127 ? Qt::black : Qt::white;
  screenOutlineRectItem->setPen(QPen(screenRectPenColor, 0));
}

void OamGraphicsScene::updateSpritePalette() {
  for(int p = 0; p < N_PALETTES; p++) {
    for(int c = 1; c < 16; c++) {
      spritePalette[p * 16 + c] = rgbFromCgram(128 + p * 16 + c);
    }
    spritePalette[p * 16] = qRgba(0, 0, 0, 0);
  }
}

void OamGraphicsScene::updateBackgroundColors() {
  screenBackgroundColor = rgbFromCgram(0);

  for(int p = 0; p < N_PALETTES; p++) {
    backgroundColors[p] = rgbFromCgram(128 + p * 16);
  }
}

void OamGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  // Do not deselect items when right mouse button is pressed
  if(event->button() == Qt::RightButton) {
    event->accept();
    return;
  }

  QGraphicsScene::mousePressEvent(event);
}

void OamGraphicsScene::resizeImageBuffer(QImage& imageBuffer, const QSize& size) {
  if(imageBuffer.size() != size) {
    QImage newImage(size, QImage::Format_ARGB32);
    imageBuffer.swap(newImage);
  }
}

void OamGraphicsScene::drawObject(QImage& buffer, const OamObject& obj) {
  // assumes buffer is the same size as the object

  const QSize objSize = dataModel->sizeOfObject(obj);

  const QRgb* pal = spritePalette + (obj.palette % N_PALETTES) * 16;
  const uint8_t *objTileset = SNES::memory::vram.data() + SNES::ppu.oam_tile_addr(obj.table);

  for(unsigned ty = 0; ty < objSize.height() / 8; ty++) {
    for(unsigned tx = 0; tx < objSize.width() / 8; tx++) {
      const unsigned cx = (obj.character + tx) & 0x00f;
      const unsigned cy = (obj.character / 16) + ty;
      const uint8_t* tile = objTileset + cy * 512 + cx * 32;

      for(unsigned py = 0; py < 8; py++) {
        const unsigned iy = ty * 8 + py;
        const unsigned fiy = (obj.vFlip == false) ? iy : objSize.height() - iy - 1;

        QRgb* dest = (QRgb*)buffer.scanLine(fiy);

        uint8_t d0 = tile[ 0];
        uint8_t d1 = tile[ 1];
        uint8_t d2 = tile[16];
        uint8_t d3 = tile[17];
        for(unsigned px = 0; px < 8; px++) {
          const unsigned ix = tx * 8 + px;
          const unsigned fix = obj.hFlip == false ? ix : objSize.width() - ix - 1;

          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          pixel |= (d2 & (0x80 >> px)) ? 4 : 0;
          pixel |= (d3 & (0x80 >> px)) ? 8 : 0;
          dest[fix] = pal[pixel];
        }
        tile += 2;
      }
    }
  }
}
