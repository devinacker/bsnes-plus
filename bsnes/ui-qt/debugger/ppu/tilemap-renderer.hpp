struct TilemapRenderer {
  enum BitDepth { BPP8, BPP4, BPP2, MODE7, NONE };

  QRgb palette[256];
  QImage image;

  QRgb customBackgroundColor;

  unsigned screenMode;
  unsigned layer;

  BitDepth bitDepth;
  unsigned tileAddr;
  unsigned screenAddr;

  bool screenSizeX;
  bool screenSizeY;
  bool tileSize;

  bool overrideBackgroundColor;

public:
  TilemapRenderer();

  void updateBitDepth();
  void loadScreenMode();
  void loadTilemapSettings();

  unsigned nLayersInMode() const;
  unsigned tileSizePx() const;

  void buildPalette();

  void drawTilemap();

private:
  void initImage(unsigned width, unsigned height);
  void invalidateImage();

  void drawMap(unsigned mapAddr, unsigned startX, unsigned startY);
  void drawMapTile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* map);
  void draw8pxTile(QRgb* imgBits, const unsigned wordsPerScanline, unsigned c, uint8_t pal, bool hFlip, bool vFlip);

  void drawMode7Tilemap();
  void drawMode7Tile(QRgb* imgBits, const unsigned wordsPerScanline, unsigned c);
};
