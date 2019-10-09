struct TilemapRenderer : public BaseRenderer {
  unsigned screenMode;
  unsigned layer;

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

  void drawTilemap();

  unsigned characterAddress(unsigned c) const;

private:
  void drawMap(unsigned mapAddr, unsigned startX, unsigned startY);
  void drawMapTile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* map);
  void drawMap8pxTile(QRgb* imgBits, const unsigned wordsPerScanline, unsigned c, unsigned palOffset, bool hFlip, bool vFlip);

  void drawMode7Tilemap();
};
