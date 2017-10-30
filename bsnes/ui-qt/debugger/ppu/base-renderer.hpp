struct BaseRenderer {
  enum BitDepth { BPP8, BPP4, BPP2, MODE7, NONE };

  QRgb customBackgroundColor;
  QRgb palette[256];
  QImage image;

  BitDepth bitDepth;

  bool overrideBackgroundColor;

public:
  BaseRenderer();

  unsigned bytesInbetweenTiles() const;
  unsigned colorsPerTile() const;

  static BitDepth bitDepthForLayer(unsigned screenMode, unsigned layer);

protected:
  void buildPalette();

  void initImage(unsigned width, unsigned height);
  void invalidateImage();

  void draw8pxTile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* tile, unsigned palOffset, bool hFlip, bool vFlip);

  void drawMode7Tile(QRgb* imgBits, const unsigned wordsPerScanline, const uint8_t* tile);
};
