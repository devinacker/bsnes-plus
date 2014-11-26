class Screen {
  uint16 *output;

  struct Regs {
    bool addsub_mode;
    bool direct_color;

    bool color_mode;
    bool color_halve;
    bool bg1_color_enable;
    bool bg2_color_enable;
    bool bg3_color_enable;
    bool bg4_color_enable;
    bool oam_color_enable;
    bool back_color_enable;

    uint16 color;
  } regs;

  struct Math {
    struct Layer {
      uint16 color;
      bool color_enable;
    } main, sub;
    bool transparent;
    bool addsub_mode;
    bool color_halve;
  } math;

  void scanline();
  void run();
  void reset();

  uint16 light_table[16][32768];
  uint16 get_pixel_sub(bool hires);
  uint16 get_pixel_main();
  uint16 addsub(unsigned x, unsigned y);
  uint16 get_color(unsigned palette);
  uint16 get_direct_color(unsigned palette, unsigned tile);

  void serialize(serializer&);
  Screen(PPU &self);

  PPU &self;
  friend class PPU;
};
