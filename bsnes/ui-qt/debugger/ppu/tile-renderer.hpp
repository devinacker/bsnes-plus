struct TileRenderer : public BaseRenderer {
  enum Source { VRAM, CPU_BUS };

  Source source;
  unsigned cpuAddress;

  unsigned width;

  unsigned paletteOffset;
  bool useCgramPalette;

public:
  TileRenderer();

  unsigned nTiles() const;

  void draw();

private:
  void buildCgramPalette();
  void buildBlackWhitePalette();

  void drawVramTileset();
  void drawMode7Tileset();
  void drawCpuBusTiles();
};
