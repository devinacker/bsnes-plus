
TilemapRenderer::TilemapRenderer()
{
  screenMode = 0;
  layer = 0;
  tileAddr = 0;
  screenAddr = 0;
  bitDepth = BitDepth::NONE;
  screenSizeX = false;
  screenSizeY = false;
  tileSize = false;
}

void TilemapRenderer::updateBitDepth() {
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

  layer = layer & 3;
  screenMode = screenMode & 7;
  bitDepth = map[screenMode][layer];
}

unsigned TilemapRenderer::nLayersInMode() const {
  const static unsigned layers[8] = { 4, 3, 2, 2, 2, 2, 1, 1 };

  return layers[screenMode & 7];
}

void TilemapRenderer::loadScreenMode() {
  screenMode = SNES::ppu.bg_mode() & 7;
}

void TilemapRenderer::loadTilemapSettings() {
  layer = layer & 3;

  updateBitDepth();

  if(screenMode < 7) {
    unsigned ss = SNES::ppu.bg_screen_size(layer);

    screenAddr = SNES::ppu.bg_screen_addr(layer);
    tileAddr = SNES::ppu.bg_tile_addr(layer);
    screenSizeX = ss & 1;
    screenSizeY = ss & 2;
    tileSize = SNES::ppu.bg_tile_size(layer);
  }
  else {
    screenAddr = 0;
    tileAddr = 0;
    screenSizeX = false;
    screenSizeY = false;
    tileSize = false;
  }
}

void TilemapRenderer::buildPalette() {
  if(SNES::cartridge.loaded()) {
    for(unsigned i = 0; i < 256; i++) {
      palette[i] = rgbFromCgram(i);
    }
  }
}

QImage TilemapRenderer::drawTilemap() {
  if(bitDepth == BitDepth::MODE7) return drawMode7Tilemap();
  if(bitDepth == BitDepth::NONE) return QImage();

  unsigned mapSize = tileSize ? 512 : 256;
  unsigned width = mapSize * (screenSizeX + 1);
  unsigned height = mapSize * (screenSizeY + 1);

  QImage image(width, height, QImage::Format_RGB32);

  unsigned addr = screenAddr;
  for(unsigned y = 0; y < height; y += mapSize) {
    for(unsigned x = 0; x < width; x += mapSize) {
      drawMap(image, addr, x, y);
      addr += 0x800;
    }
  }

  return image;
}

void TilemapRenderer::drawMap(QImage& image, unsigned mapAddr, unsigned startX, unsigned startY) {
  unsigned ts = tileSize ? 16 : 8;
  unsigned wordsPerScanline = image.bytesPerLine() / 4;

  mapAddr = mapAddr & 0xf800;
  const uint8_t *map = SNES::memory::vram.data() + mapAddr;

  for(unsigned ty = 0; ty < 32; ty++) {
    uint32_t* imgBits = (uint32_t*)image.scanLine(startY + ty * ts) + startX;

    for(unsigned tx = 0; tx < 32; tx++) {
      drawMapTile(imgBits, wordsPerScanline, map);
      imgBits += ts;
      map += 2;
    }
  }
}

void TilemapRenderer::drawMapTile(uint32_t* imgBits, const unsigned wordsPerScanline, const uint8_t* map) {
  unsigned ts = tileSize ? 16 : 8;
  uint16_t tile = map[0] | (map[1] << 8);

  unsigned c = tile & 0x03ff;
  uint8_t pal = (tile >> 10) & 7;
  bool hFlip = tile & 0x4000;
  bool vFlip = tile & 0x8000;

  switch(bitDepth) {
    case BitDepth::BPP8: pal = 0;        break;
    case BitDepth::BPP4: pal = pal * 16; break;
    case BitDepth::BPP2: pal = pal *  4 + ((screenMode == 0) ? layer * 32 : 0); break;
  }

  if(tileSize == false) {
    draw8pxTile(imgBits, wordsPerScanline, c, pal, hFlip, vFlip);

  } else {
    // 16x16 tile
    unsigned c1 = c;
    unsigned c2 = (c & 0x3f0) | ((c + 1) & 0x00f);
    if (hFlip) { swap(c1, c2); }

    unsigned c3 = c1 + 0x010;
    unsigned c4 = c2 + 0x010;
    if (vFlip) { swap(c1, c3); swap(c2, c4); }

    uint32_t* row2Bits = imgBits + wordsPerScanline * 8;
    draw8pxTile(imgBits  + 0, wordsPerScanline, c1, pal, hFlip, vFlip);
    draw8pxTile(imgBits  + 8, wordsPerScanline, c2, pal, hFlip, vFlip);
    draw8pxTile(row2Bits + 0, wordsPerScanline, c3, pal, hFlip, vFlip);
    draw8pxTile(row2Bits + 8, wordsPerScanline, c4, pal, hFlip, vFlip);
  }
}

void TilemapRenderer::draw8pxTile(uint32_t* imgBits, const unsigned wordsPerScanline, unsigned c, uint8_t pal, bool hFlip, bool vFlip) {
  uint8_t data[8];

  unsigned addr = 0;
  switch(bitDepth) {
    case BitDepth::BPP8: addr += (tileAddr + c * 64) & 0xffc0; break;
    case BitDepth::BPP4: addr += (tileAddr + c * 32) & 0xffe0; break;
    case BitDepth::BPP2: addr += (tileAddr + c * 16) & 0xfff0; break;
  }

  const uint8_t *tile = SNES::memory::vram.data() + addr;

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
        imgBits[fpx] = palette[(pal + pixel) & 0xff];
      } else {
        imgBits[fpx] = palette[0];
      }
    }

    imgBits += wordsPerScanline;
  }
}

QImage TilemapRenderer::drawMode7Tilemap() {
  QImage image(1024, 1024, QImage::Format_RGB32);

  uint32_t* scanline = (uint32_t*)image.scanLine(0);
  unsigned wordsPerScanline = image.bytesPerLine() / 4;

  const uint8_t *map = SNES::memory::vram.data();

  for(unsigned ty = 0; ty < 128; ty++) {
    uint32_t* imgBits = scanline;
    scanline += wordsPerScanline * 8;

    for(unsigned tx = 0; tx < 128; tx++) {
      unsigned c = *map;
      map += 2;

      drawMode7Tile(imgBits, wordsPerScanline, c);
      imgBits += 8;
    }
  }

  return image;
}

void TilemapRenderer::drawMode7Tile(uint32_t* imgBits, const unsigned wordsPerScanline, unsigned c) {
  const uint8_t *tile = SNES::memory::vram.data() + c * 128 + 1;

  for(unsigned py = 0; py < 8; py++) {
    for(unsigned px = 0; px < 8; px++) {
      imgBits[px] = palette[*tile];
      tile +=2;
    }
    imgBits += wordsPerScanline;
  }
}
