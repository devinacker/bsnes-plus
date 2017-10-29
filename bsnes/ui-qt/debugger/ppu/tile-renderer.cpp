
TileRenderer::TileRenderer()
    : BaseRenderer()
{
  bitDepth = BitDepth::BPP4;
  width = 16;

  paletteOffset = 0;
  useCgramPalette = false;
}

unsigned TileRenderer::nTiles() const {
  switch (bitDepth) {
    case BitDepth::BPP8: return 1024;
    case BitDepth::BPP4: return 2048;
    case BitDepth::BPP2: return 4096;
    case BitDepth::MODE7: return 256;
  }
  return 0;
}

void TileRenderer::buildCgramPalette() {
  const unsigned nColors = colorsPerTile();

  paletteOffset &= 0xff;

  unsigned start = paletteOffset & (0xff - nColors + 1);
  assert(start + nColors < 256);

  for(unsigned i = 0; i < nColors; i++) {
    palette[i] = rgbFromCgram(start + i);
  }
}

void TileRenderer::buildBlackWhitePalette() {
  const unsigned nColors = colorsPerTile();

  uint8_t delta = 255 / (nColors - 1);

  uint8_t pixel = 0;
  for(unsigned i = 0; i < nColors; i++) {
    palette[i] = qRgb(pixel, pixel, pixel);
    pixel += delta;
  }
}

void TileRenderer::draw() {
  if(!SNES::cartridge.loaded()) { invalidateImage(); return; }
  if(bitDepth == BitDepth::NONE) { invalidateImage(); return; }

  if(useCgramPalette) {
    buildCgramPalette();
  } else {
    buildBlackWhitePalette();
  }

  if(width < 8) width = 8;
  if(width > 64) width = 64;

  if(bitDepth == BitDepth::MODE7) { drawMode7Tileset(); return; }

  drawVramTileset();
}

void TileRenderer::drawVramTileset() {
  const unsigned height = (nTiles() + width - 1) / width;

  initImage(width * 8, height * 8);

  QRgb* scanline = (QRgb*)image.scanLine(0);
  const unsigned wordsPerScanline = image.bytesPerLine() / 4;
  const unsigned bytesPerTile = bytesInbetweenTiles();

  const uint8_t *tile = SNES::memory::vram.data();
  const uint8_t *tileEnd = tile + SNES::memory::vram.size();

  for(unsigned y = 0; y < height; y++) {
    QRgb* imgBits = scanline;
    scanline += wordsPerScanline * 8;

    for(unsigned x = 0; x < width; x++) {
      if(tile < tileEnd) {
        draw8pxTile(imgBits, wordsPerScanline, tile, 0, 0, 0);

        imgBits += 8;
        tile += bytesPerTile;
      }
    }
  }
}

void TileRenderer::drawMode7Tileset() {
  const unsigned height = (256 + width - 1) / width;

  initImage(width * 8, height * 8);

  QRgb* scanline = (QRgb*)image.scanLine(0);
  const unsigned wordsPerScanline = image.bytesPerLine() / 4;

  const uint8_t *tile = SNES::memory::vram.data() + 1;
  const uint8_t *tileEnd = tile + 256 * 128;

  for(unsigned y = 0; y < height; y++) {
    QRgb* imgBits = scanline;
    scanline += wordsPerScanline * 8;

    for(unsigned x = 0; x < width; x++) {
      if(tile < tileEnd) {
        drawMode7Tile(imgBits, wordsPerScanline, tile);

        tile += 128;
        imgBits += 8;
      }
    }
  }
}
