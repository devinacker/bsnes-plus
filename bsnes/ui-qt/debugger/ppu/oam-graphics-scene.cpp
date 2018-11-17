#include "oam-graphics-scene.moc"

OamGraphicsScene::OamGraphicsScene(OamDataModel* dataModel, QObject* parent)
  : QGraphicsScene(parent)
  , dataModel(dataModel)
  , smallImageBuffer()
  , largeImageBuffer()
  , spritePalette()
  , objects()
{
  const QString toolTipStr = QString::fromLatin1("Sprite %1");

  objects.reserve(N_OBJECTS);

  for(int i = 0; i < N_OBJECTS; i++) {
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem;

    item->setData(IdRole, i);
    item->setToolTip(toolTipStr.arg(i));

    item->setZValue(N_OBJECTS - i);

    this->addItem(item);

    objects.append(item);
  }
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

  resizeImageBuffer(smallImageBuffer, dataModel->objectSizes().small);
  resizeImageBuffer(largeImageBuffer, dataModel->objectSizes().large);

  for(int id = 0; id < N_OBJECTS; id++) {
    QGraphicsPixmapItem* item = objects.at(id);
    const OamObject& obj = dataModel->oamObject(id);

    item->setPos(obj.xpos, obj.ypos);

    if(obj.size == false) {
      drawObject(smallImageBuffer, obj);
      item->setPixmap(QPixmap::fromImage(smallImageBuffer));
    }
    else {
      drawObject(largeImageBuffer, obj);
      item->setPixmap(QPixmap::fromImage(largeImageBuffer));
    }
  }
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
  for(int p = 0; p < N_PALETTES; p++) {
    backgroundColors[p] = rgbFromCgram(128 + p * 16);
  }
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
