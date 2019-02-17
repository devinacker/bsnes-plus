
TilemapRenderer::TilemapRenderer()
    : BaseRenderer()
{
  screenMode = 0;
  layer = 0;
  tileAddr = 0;
  screenAddr = 0;
  screenSizeX = false;
  screenSizeY = false;
  tileSize = false;
}

void TilemapRenderer::updateBitDepth() {
  layer = layer & 3;
  screenMode = screenMode & 7;
  bitDepth = bitDepthForLayer(screenMode, layer);
}

unsigned TilemapRenderer::nLayersInMode() const {
  const static unsigned layers[8] = { 4, 3, 2, 2, 2, 2, 1, 2 };

  return layers[screenMode & 7];
}

unsigned TilemapRenderer::tileSizePx() const {
  if(!isMode7()) {
    return tileSize ? 16 : 8;
  } else {
    return 8;
  }
}

void TilemapRenderer::loadScreenMode() {
  screenMode = SNES::ppu.bg_mode() & 7;
  if(screenMode == 7) {
      layer = SNES::ppu.mode7_extbg();
  }
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

void TilemapRenderer::drawTilemap() {
  buildPalette();

  if(isMode7()) { drawMode7Tilemap(); return; }
  if(bitDepth == BitDepth::NONE) { invalidateImage(); return; }

  unsigned mapSize = tileSize ? 512 : 256;
  unsigned width = mapSize * (screenSizeX + 1);
  unsigned height = mapSize * (screenSizeY + 1);

  initImage(width, height);

  unsigned addr = screenAddr;
  for(unsigned y = 0; y < height; y += mapSize) {
    for(unsigned x = 0; x < width; x += mapSize) {
      drawMap(addr, x, y);
      addr += 0x800;
    }
  }
}

void TilemapRenderer::drawMap(unsigned mapAddr, unsigned startX, unsigned startY) {
  unsigned ts = tileSize ? 16 : 8;
  unsigned wordsPerScanline = image.bytesPerLine() / 4;

  mapAddr = mapAddr & 0xf800;
  const uint8_t *map = SNES::memory::vram.data() + mapAddr;

  for(unsigned ty = 0; ty < 32; ty++) {
    QRgb* imgBits = (QRgb*)image.scanLine(startY + ty * ts) + startX;

    for(unsigned tx = 0; tx < 32; tx++) {
      drawMapTile(imgBits, wordsPerScanline, map);
      imgBits += ts;
      map += 2;
    }
  }
}

void TilemapRenderer::drawMapTile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* map) {
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
    drawMap8pxTile(imgBits, wordsPerScanline, c, pal, hFlip, vFlip);

  } else {
    // 16x16 tile
    unsigned c1 = c;
    unsigned c2 = (c & 0x3f0) | ((c + 1) & 0x00f);
    if (hFlip) { swap(c1, c2); }

    unsigned c3 = c1 + 0x010;
    unsigned c4 = c2 + 0x010;
    if (vFlip) { swap(c1, c3); swap(c2, c4); }

    QRgb* row2Bits = imgBits + wordsPerScanline * 8;
    drawMap8pxTile(imgBits  + 0, wordsPerScanline, c1, pal, hFlip, vFlip);
    drawMap8pxTile(imgBits  + 8, wordsPerScanline, c2, pal, hFlip, vFlip);
    drawMap8pxTile(row2Bits + 0, wordsPerScanline, c3, pal, hFlip, vFlip);
    drawMap8pxTile(row2Bits + 8, wordsPerScanline, c4, pal, hFlip, vFlip);
  }
}

unsigned TilemapRenderer::characterAddress(unsigned c) const {
  switch(bitDepth) {
    case BitDepth::BPP8:        return (tileAddr + c * 64) & 0xffc0;
    case BitDepth::BPP4:        return (tileAddr + c * 32) & 0xffe0;
    case BitDepth::BPP2:        return (tileAddr + c * 16) & 0xfff0;
    case BitDepth::MODE7:       return (c & 0xff) * 128 + 1;
    case BitDepth::MODE7_EXTBG: return (c & 0xff) * 128 + 1;
  }
  return 0;
}

void TilemapRenderer::drawMap8pxTile(QRgb* imgBits, const unsigned wordsPerScanline, unsigned c, unsigned palOffset, bool hFlip, bool vFlip) {
  unsigned addr = characterAddress(c);

  const uint8_t *tile = SNES::memory::vram.data() + addr;

  draw8pxTile(imgBits, wordsPerScanline, tile, palOffset, hFlip, vFlip);
}

void TilemapRenderer::drawMode7Tilemap() {
  initImage(1024, 1024);

  QRgb* scanline = (QRgb*)image.scanLine(0);
  unsigned wordsPerScanline = image.bytesPerLine() / 4;

  const uint8_t *map = SNES::memory::vram.data();

  for(unsigned ty = 0; ty < 128; ty++) {
    QRgb* imgBits = scanline;
    scanline += wordsPerScanline * 8;

    for(unsigned tx = 0; tx < 128; tx++) {
      unsigned c = *map;
      const uint8_t *tile = SNES::memory::vram.data() + c * 128 + 1;

      drawMode7Tile(imgBits, wordsPerScanline, tile);

      map += 2;
      imgBits += 8;
    }
  }
}
