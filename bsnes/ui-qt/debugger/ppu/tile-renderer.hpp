struct TileRenderer : public BaseRenderer {
  enum Source { VRAM, CPU_BUS };

  Source source;
  unsigned address;

  unsigned width;

  unsigned paletteOffset;
  bool useCgramPalette;

public:
  TileRenderer();

  unsigned addressMask() const;
  unsigned nTiles() const;

  void draw();

private:
  void buildCgramPalette();
  void buildBlackWhitePalette();

  void drawVramTileset();
  void drawMode7Tileset();
  void drawCpuBusTiles();
};
