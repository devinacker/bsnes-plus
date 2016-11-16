struct TilemapRenderer {
  enum BitDepth { BPP8, BPP4, BPP2 };

  uint32_t palette[256];

  unsigned tileAddr;
  unsigned screenAddr;

  BitDepth bitDepth;
  bool screenSizeX;
  bool screenSizeY;
  bool tileSize;

public:
  TilemapRenderer();

  void buildPalette();

  QImage drawTilemap();

private:
  void drawMap(QImage& image, unsigned mapAddr, unsigned startX, unsigned startY);
  void drawMapScanline(uint32_t* scanline, const unsigned py, const uint8_t* map);
  void drawSliver(uint32_t* imgBits, unsigned c, unsigned fpy, const uint8_t pal, bool hFlip);
};
