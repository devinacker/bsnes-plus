struct TileRenderer : public BaseRenderer {
  enum Source { VRAM, CPU_BUS, CART_ROM, CART_RAM, SA1_BUS, SFX_BUS };

  Source source;
  unsigned address;

  unsigned width;

  unsigned paletteOffset;
  bool useCgramPalette;

public:
  TileRenderer();

  unsigned addressMask() const;
  unsigned nTiles() const;
  unsigned maxAddress() const;

  void draw();

private:
  void buildCgramPalette();
  void buildBlackWhitePalette();

  void drawVramTileset();
  void drawMode7Tileset();
  void drawMemorySourceTiles();
};
