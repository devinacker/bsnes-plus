
BaseRenderer::BaseRenderer()
{
  customBackgroundColor = 0;
  bitDepth = BitDepth::NONE;
  overrideBackgroundColor = false;
}

BaseRenderer::BitDepth BaseRenderer::bitDepthForLayer(unsigned screenMode, unsigned layer) {
  static const BitDepth map[8][4] = {
    { BPP2, BPP2, BPP2, BPP2},
    { BPP4, BPP4, BPP2, NONE},
    { BPP4, BPP4, NONE, NONE},
    { BPP8, BPP4, NONE, NONE},
    { BPP8, BPP2, NONE, NONE},
    { BPP4, BPP2, NONE, NONE},
    { BPP4, NONE, NONE, NONE},
    { MODE7, MODE7, MODE7, MODE7}
  };

  return map[screenMode & 7][layer & 3];
}

unsigned BaseRenderer::bytesInbetweenTiles() const {
  switch (bitDepth) {
    case BitDepth::BPP8: return 64;
    case BitDepth::BPP4: return 32;
    case BitDepth::BPP2: return 16;
    case BitDepth::MODE7: return 128;
  }
  return 0;
}

unsigned BaseRenderer::colorsPerTile() const {
  switch (bitDepth) {
    case BitDepth::BPP8: return 256;
    case BitDepth::BPP4: return 16;
    case BitDepth::BPP2: return 4;
    case BitDepth::MODE7: return 256;
  }
  return 0;
}

void BaseRenderer::buildPalette() {
  if(SNES::cartridge.loaded()) {
    for(unsigned i = 0; i < 256; i++) {
      palette[i] = rgbFromCgram(i);
    }
  }
}

void BaseRenderer::initImage(unsigned width, unsigned height)
{
  QImage::Format format = QImage::Format_RGB32;
  if(overrideBackgroundColor) {
    if(qAlpha(customBackgroundColor) != 0xff) format = QImage::Format_ARGB32;
    if(customBackgroundColor == 0) format = QImage::Format_ARGB32_Premultiplied;
  }

  if(image.width() != width || image.height() != height || image.format() != format) {
    image = QImage(width, height, format);
  }

  if(overrideBackgroundColor) {
    image.fill(customBackgroundColor);
  } else {
    image.fill(palette[0]);
  }
}

void BaseRenderer::invalidateImage()
{
  image = QImage();
}

void BaseRenderer::draw8pxTile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* tile, unsigned palOffset, bool hFlip, bool vFlip) {
  uint8_t data[8];

  for(unsigned py = 0; py < 8; py++) {
    unsigned fpy = (vFlip == false) ? py : 7 - py;
    const uint8_t *sliver = tile + fpy * 2;

    switch(bitDepth) {
    case BitDepth::BPP8:
      data[4] = sliver[32];
      data[5] = sliver[33];
      data[6] = sliver[48];
      data[7] = sliver[49];
      //fall through
    case BitDepth::BPP4:
      data[2] = sliver[16];
      data[3] = sliver[17];
      //fall through
    case BitDepth::BPP2:
      data[0] = sliver[ 0];
      data[1] = sliver[ 1];
    }

    for(unsigned px = 0; px < 8; px++) {
      unsigned fpx = hFlip == false ? px : 7 - px;

      uint8_t pixel = 0;
      switch(bitDepth) {
      case BitDepth::BPP8:
        pixel |= (data[7] & (0x80 >> px)) ? 0x80 : 0;
        pixel |= (data[6] & (0x80 >> px)) ? 0x40 : 0;
        pixel |= (data[5] & (0x80 >> px)) ? 0x20 : 0;
        pixel |= (data[4] & (0x80 >> px)) ? 0x10 : 0;
        //fall through
      case BitDepth::BPP4:
        pixel |= (data[3] & (0x80 >> px)) ? 0x08 : 0;
        pixel |= (data[2] & (0x80 >> px)) ? 0x04 : 0;
        //fall through
      case BitDepth::BPP2:
        pixel |= (data[1] & (0x80 >> px)) ? 0x02 : 0;
        pixel |= (data[0] & (0x80 >> px)) ? 0x01 : 0;
      }

      if (pixel != 0) {
        imgBits[fpx] = palette[(palOffset + pixel) & 0xff];
      }
    }

    imgBits += wordsPerScanline;
  }
}

void BaseRenderer::drawMode7Tile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* tile) {
  for(unsigned py = 0; py < 8; py++) {
    for(unsigned px = 0; px < 8; px++) {
      if(*tile != 0) imgBits[px] = palette[*tile];
      tile +=2;
    }
    imgBits += wordsPerScanline;
  }
}
