
TileRenderer::TileRenderer()
    : BaseRenderer()
{
  source = VRAM;
  address = 0;

  bitDepth = BitDepth::BPP4;
  width = 16;

  paletteOffset = 0;
  useCgramPalette = false;
}

unsigned TileRenderer::addressMask() const {
  if(source != Source::VRAM) return 0xffffff;

  switch (bitDepth) {
    case BitDepth::BPP8: return 0xffc0;
    case BitDepth::BPP4: return 0xffe0;
    case BitDepth::BPP2: return 0xfff0;
    case BitDepth::MODE7: return 0;
    case BitDepth::MODE7_EXTBG: return 0;
  }
  return 0;
}

unsigned TileRenderer::nTiles() const {
  if(source != Source::VRAM) return int(255 / width + 1) * width;

  if(isMode7()) return 256;

  unsigned a = address & addressMask();
  return (0x10000 - a) / bytesInbetweenTiles();
}

unsigned TileRenderer::maxAddress() const {
  switch(source) {
    case Source::VRAM:     return 64 * 1024;
    case Source::CPU_BUS:  return 16 * 1024 * 1024;
    case Source::CART_ROM: return SNES::memory::cartrom.size();
    case Source::CART_RAM: return SNES::memory::cartram.size();
    case Source::SA1_BUS:  return SNES::cartridge.has_sa1()    ? 16 * 1024 * 1024 : 0;
    case Source::SFX_BUS:  return SNES::cartridge.has_superfx() ? 8 * 1024 * 1024 : 0;
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

  if(isMode7()) { drawMode7Tileset(); return; }

  if(source == Source::VRAM) { drawVramTileset(); return; }

  drawMemorySourceTiles();
}

void TileRenderer::drawVramTileset() {
  source = Source::VRAM;
  address &= addressMask();

  const unsigned height = (nTiles() + width - 1) / width;

  initImage(width * 8, height * 8);

  QRgb* scanline = (QRgb*)image.scanLine(0);
  const unsigned wordsPerScanline = image.bytesPerLine() / 4;
  const unsigned bytesPerTile = bytesInbetweenTiles();

  const uint8_t *tile = SNES::memory::vram.data() + address;
  const uint8_t *tileEnd = SNES::memory::vram.data() + SNES::memory::vram.size();

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
  source = Source::VRAM;
  address = 0;

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

void TileRenderer::drawMemorySourceTiles() {
  typedef SNES::Debugger::MemorySource MemorySource;

  if(source == Source::VRAM) source = Source::CPU_BUS;
  address &= 0xffffff;

  MemorySource memSource = MemorySource::CPUBus;
  switch(source) {
    case Source::CPU_BUS:  memSource = MemorySource::CPUBus;  break;
    case Source::CART_ROM: memSource = MemorySource::CartROM; break;
    case Source::CART_RAM: memSource = MemorySource::CartRAM; break;
    case Source::SA1_BUS:  memSource = MemorySource::SA1Bus;  break;
    case Source::SFX_BUS:  memSource = MemorySource::SFXBus;  break;
  }

  const unsigned height = nTiles() / width;

  initImage(width * 8, height * 8);

  QRgb* scanline = (QRgb*)image.scanLine(0);
  const unsigned wordsPerScanline = image.bytesPerLine() / 4;
  const unsigned bytesPerTile = bytesInbetweenTiles();

  unsigned addr = address;
  uint8_t tile[64];

  if(bytesPerTile > 64) return;


  SNES::debugger.bus_access = true;

  for(unsigned y = 0; y < height; y++) {
    QRgb* imgBits = scanline;
    scanline += wordsPerScanline * 8;

    for(unsigned x = 0; x < width; x++) {
      for(unsigned i = 0; i < bytesPerTile; i++) {
        tile[i] = SNES::debugger.read(memSource, addr);
        addr++;
      }
      draw8pxTile(imgBits, wordsPerScanline, tile, 0, 0, 0);
      imgBits += 8;
    }
  }

  SNES::debugger.bus_access = false;
}
