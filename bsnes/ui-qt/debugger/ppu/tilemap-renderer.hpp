struct TilemapRenderer {
  enum BitDepth { BPP8, BPP4, BPP2, MODE7, NONE };

  QRgb palette[256];

  unsigned screenMode;
  unsigned layer;

  BitDepth bitDepth;
  unsigned tileAddr;
  unsigned screenAddr;

  bool screenSizeX;
  bool screenSizeY;
  bool tileSize;

public:
  TilemapRenderer();

  void updateBitDepth();
  void loadScreenMode();
  void loadTilemapSettings();

  unsigned nLayersInMode() const;
  unsigned tileSizePx() const;

  void buildPalette();

  QImage drawTilemap();

private:
  void drawMap(QImage& image, unsigned mapAddr, unsigned startX, unsigned startY);
  void drawMapTile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* map);
  void draw8pxTile(QRgb* imgBits, const unsigned wordsPerScanline, unsigned c, uint8_t pal, bool hFlip, bool vFlip);

  QImage drawMode7Tilemap();
  void drawMode7Tile(QRgb* imgBits, const unsigned wordsPerScanline, unsigned c);
};
