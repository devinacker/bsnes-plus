struct TileRenderer : public BaseRenderer {
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
};
