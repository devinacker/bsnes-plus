#ifdef PPU_CPP

uint8 PPUDebugger::vram_read(unsigned addr) {
  uint8 data = PPU::vram_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::VRAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void PPUDebugger::vram_write(unsigned addr, uint8 data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::VRAM, Debugger::Breakpoint::Mode::Write, addr, data);
  PPU::vram_write(addr, data);
}

uint8 PPUDebugger::oam_read(unsigned addr) {
  uint8 data = PPU::oam_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::OAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void PPUDebugger::oam_write(unsigned addr, uint8 data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::OAM, Debugger::Breakpoint::Mode::Write, addr, data);
  PPU::oam_write(addr, data);
}

uint8 PPUDebugger::cgram_read(unsigned addr) {
  uint8 data = PPU::cgram_read(addr);
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CGRAM, Debugger::Breakpoint::Mode::Read, addr, data);
  return data;
}

void PPUDebugger::cgram_write(unsigned addr, uint8 data) {
  debugger.breakpoint_test(Debugger::Breakpoint::Source::CGRAM, Debugger::Breakpoint::Mode::Write, addr, data);
  PPU::cgram_write(addr, data);
}

uint8 PPUDebugger::bg_mode() const {
  return (uint8)regs.bgmode;
}

uint16 PPUDebugger::bg_screen_addr(unsigned index) const {
  switch (index) {
  case 0: return bg1.regs.screen_addr;
  case 1: return bg2.regs.screen_addr;
  case 2: return bg3.regs.screen_addr;
  case 3: return bg4.regs.screen_addr;
  }
  
  return 0;
}

uint8 PPUDebugger::bg_screen_size(unsigned index) const {
  switch (index) {
  case 0: return bg1.regs.screen_size & 3;
  case 1: return bg2.regs.screen_size & 3;
  case 2: return bg3.regs.screen_size & 3;
  case 3: return bg4.regs.screen_size & 3;
  }
  
  return 0;
}

uint16 PPUDebugger::bg_tile_addr(unsigned index) const {
  switch (index) {
  case 0: return bg1.regs.tiledata_addr;
  case 1: return bg2.regs.tiledata_addr;
  case 2: return bg3.regs.tiledata_addr;
  case 3: return bg4.regs.tiledata_addr;
  }
  
  return 0;
}

uint8 PPUDebugger::bg_tile_size(unsigned index) const {
  switch (index) {
  case 0: return bg1.regs.tile_size;
  case 1: return bg2.regs.tile_size;
  case 2: return bg3.regs.tile_size;
  case 3: return bg4.regs.tile_size;
  }
  
  return 0;
}

uint16 PPUDebugger::oam_tile_addr(unsigned index) const {
  return index
    ? (oam.regs.tiledata_addr + (256 * 32) + (oam.regs.nameselect << 13)) & 0xffff
    : oam.regs.tiledata_addr;
}

uint8 PPUDebugger::oam_base_size() const { 
  return oam.regs.base_size; 
}

unsigned PPUDebugger::oam_first_sprite() const {
  return oam.regs.first_sprite;
}

bool PPUDebugger::mode7_extbg() const {
  return regs.mode7_extbg;
}

bool PPUDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  //internal
  item("S-PPU1 open bus", string("0x", hex<2>(regs.ppu1_mdr)));
  item("S-PPU2 open bus", string("0x", hex<2>(regs.ppu2_mdr)));

  //$2100
  item("$2100", "");
  item("Display Disable", regs.display_disable);
  item("Display Brightness", (unsigned)regs.display_brightness);

  //$2101
  item("$2101", "");
  item("OAM Base Size", (unsigned)oam.regs.base_size);
  item("OAM Name Select", (unsigned)oam.regs.nameselect);
  item("OAM Name Base Address", string("0x", hex<4>(oam.regs.tiledata_addr)));
  item("OAM Second Name Table Address", string("0x", hex<4>(oam_tile_addr(1))));

  //$2102-$2103
  item("$2102-$2103", "");
  item("OAM Base Address", string("0x", hex<4>(regs.oam_baseaddr)));
  item("OAM Priority", regs.oam_priority);

  //$2105
  item("$2105", "");
  item("BG1 Tile Size", bg1.regs.tile_size ? "16x16" : "8x8");
  item("BG2 Tile Size", bg2.regs.tile_size ? "16x16" : "8x8");
  item("BG3 Tile Size", bg3.regs.tile_size ? "16x16" : "8x8");
  item("BG4 Tile Size", bg4.regs.tile_size ? "16x16" : "8x8");
  item("BG3 Priority", regs.bg3_priority);
  item("BG Mode", (unsigned)regs.bgmode);

  //$2106
  item("$2106", "");
  item("BG1 Mosaic Size", bg1.regs.mosaic);
  item("BG2 Mosaic Size", bg2.regs.mosaic);
  item("BG3 Mosaic Size", bg3.regs.mosaic);
  item("BG4 Mosaic Size", bg4.regs.mosaic);

  // Width x Height
  static char screen_size[4][8] = { "32x32", "64x32", "32x64", "64x64" };

  //$2107
  item("$2107", "");
  item("BG1 Screen Address", string("0x", hex<4>(bg1.regs.screen_addr)));
  item("BG1 Screen Size", screen_size[bg1.regs.screen_size & 3]);

  //$2108
  item("$2108", "");
  item("BG2 Screen Address", string("0x", hex<4>(bg2.regs.screen_addr)));
  item("BG2 Screen Size", screen_size[bg2.regs.screen_size & 3]);

  //$2109
  item("$2109", "");
  item("BG3 Screen Address", string("0x", hex<4>(bg3.regs.screen_addr)));
  item("BG3 Screen Size", screen_size[bg3.regs.screen_size & 3]);

  //$210a
  item("$210a", "");
  item("BG4 Screen Address", string("0x", hex<4>(bg4.regs.screen_addr)));
  item("BG4 Screen Size", screen_size[bg4.regs.screen_size & 3]);

  //$210b
  item("$210b", "");
  item("BG1 Name Base Address", string("0x", hex<4>(bg1.regs.tiledata_addr)));
  item("BG2 Name Base Address", string("0x", hex<4>(bg2.regs.tiledata_addr)));

  //$210c
  item("$210c", "");
  item("BG3 Name Base Address", string("0x", hex<4>(bg3.regs.tiledata_addr)));
  item("BG4 Name Base Address", string("0x", hex<4>(bg4.regs.tiledata_addr)));

  //$210d
  item("$210d", "");
  item("Mode 7 Scroll H-offset", (unsigned)(regs.mode7_hoffset & 0x1fff));
  item("BG1 Scroll H-offset", (unsigned)(bg1.regs.hoffset & 0x03ff));

  //$210e
  item("$210e", "");
  item("Mode 7 Scroll V-offset", (unsigned)(regs.mode7_voffset & 0x1fff));
  item("BG1 Scroll V-offset", (unsigned)(bg1.regs.voffset & 0x03ff));

  //$210f
  item("$210f", "");
  item("BG2 Scroll H-offset", (unsigned)(bg2.regs.hoffset & 0x03ff));

  //$2110
  item("$2110", "");
  item("BG2 Scroll V-offset", (unsigned)(bg2.regs.voffset & 0x03ff));

  //$2111
  item("$2111", "");
  item("BG3 Scroll H-offset", (unsigned)(bg3.regs.hoffset & 0x03ff));

  //$2112
  item("$2112", "");
  item("BG3 Scroll V-offset", (unsigned)(bg3.regs.voffset & 0x03ff));

  //$2113
  item("$2113", "");
  item("BG4 Scroll H-offset", (unsigned)(bg4.regs.hoffset & 0x03ff));

  //$2114
  item("$2114", "");
  item("BG4 Scroll V-offset", (unsigned)(bg4.regs.voffset & 0x03ff));

  //$2115
  item("$2115", "");
  item("VRAM Increment Mode", (unsigned)regs.vram_incmode);
  item("VRAM Increment Formation", (unsigned)regs.vram_mapping);
  item("VRAM Increment Size", (unsigned)regs.vram_incsize);

  //$2116-$2117
  item("$2116-$2117", "");
  item("VRAM Address", string("0x", hex<4>(regs.vram_addr)));

  //$211a
  item("$211a", "");
  item("Mode 7 Repeat", (unsigned)regs.mode7_repeat);
  item("Mode 7 V-flip", regs.mode7_vflip);
  item("Mode 7 H-flip", regs.mode7_hflip);

  //$211b
  item("$211b", "");
  item("Mode 7 A", (unsigned)regs.m7a);

  //$211c
  item("$211c", "");
  item("Mode 7 B", (unsigned)regs.m7b);

  //$211d
  item("$211d", "");
  item("Mode 7 C", (unsigned)regs.m7c);

  //$211e
  item("$211e", "");
  item("Mode 7 D", (unsigned)regs.m7d);

  //$211f
  item("$211f", "");
  item("Mode 7 X", (unsigned)regs.m7x);

  //$2120
  item("$2120", "");
  item("Mode 7 Y", (unsigned)regs.m7y);

  //$2121
  item("$2121", "");
  item("CGRAM Address", string("0x", hex<4>(regs.cgram_addr)));

  //$2123
  item("$2123", "");
  item("BG1 Window 1 Enable", window.regs.bg1_one_enable);
  item("BG1 Window 1 Invert", window.regs.bg1_one_invert);
  item("BG1 Window 2 Enable", window.regs.bg1_two_enable);
  item("BG1 Window 2 Invert", window.regs.bg1_two_invert);
  item("BG2 Window 1 Enable", window.regs.bg2_one_enable);
  item("BG2 Window 1 Invert", window.regs.bg2_one_invert);
  item("BG2 Window 2 Enable", window.regs.bg2_two_enable);
  item("BG2 Window 2 Invert", window.regs.bg2_two_invert);

  //$2124
  item("$2124", "");
  item("BG3 Window 1 Enable", window.regs.bg3_one_enable);
  item("BG3 Window 1 Invert", window.regs.bg3_one_invert);
  item("BG3 Window 2 Enable", window.regs.bg3_two_enable);
  item("BG3 Window 2 Invert", window.regs.bg3_two_invert);
  item("BG4 Window 1 Enable", window.regs.bg4_one_enable);
  item("BG4 Window 1 Invert", window.regs.bg4_one_invert);
  item("BG4 Window 2 Enable", window.regs.bg4_two_enable);
  item("BG4 Window 2 Invert", window.regs.bg4_two_invert);

  //$2125
  item("$2125", "");
  item("OAM Window 1 Enable", window.regs.oam_one_enable);
  item("OAM Window 1 Invert", window.regs.oam_one_invert);
  item("OAM Window 2 Enable", window.regs.oam_two_enable);
  item("OAM Window 2 Invert", window.regs.oam_two_invert);
  item("Color Window 1 Enable", window.regs.col_one_enable);
  item("Color Window 1 Invert", window.regs.col_one_invert);
  item("Color Window 2 Enable", window.regs.col_two_enable);
  item("Color Window 2 Invert", window.regs.col_two_invert);

  //$2126
  item("$2126", "");
  item("Window 1 Left", (unsigned)window.regs.one_left);

  //$2127
  item("$2127", "");
  item("Window 1 Right", (unsigned)window.regs.one_right);

  //$2128
  item("$2128", "");
  item("Window 2 Left", (unsigned)window.regs.two_left);

  //$2129
  item("$2129", "");
  item("Window 2 Right", (unsigned)window.regs.two_right);

  static char window_mask_mode[4][8] = { "OR", "AND", "XOR", "XNOR" };

  //$212a
  item("$212a", "");
  item("BG1 Window Mask", window_mask_mode[window.regs.bg1_mask & 3]);
  item("BG2 Window Mask", window_mask_mode[window.regs.bg2_mask & 3]);
  item("BG3 Window Mask", window_mask_mode[window.regs.bg3_mask & 3]);
  item("BG4 Window Mask", window_mask_mode[window.regs.bg4_mask & 3]);

  //$212b
  item("$212b", "");
  item("OAM Window Mask", window_mask_mode[window.regs.oam_mask & 3]);
  item("Color Window Mask", window_mask_mode[window.regs.col_mask & 3]);

  //$212c
  item("$212c", "");
  item("BG1 Mainscreen Enable", bg1.regs.main_enable);
  item("BG2 Mainscreen Enable", bg2.regs.main_enable);
  item("BG3 Mainscreen Enable", bg3.regs.main_enable);
  item("BG4 Mainscreen Enable", bg4.regs.main_enable);
  item("OAM Mainscreen Enable", oam.regs.main_enable);

  //$212d
  item("$212d", "");
  item("BG1 Subscreen Enable", bg1.regs.sub_enable);
  item("BG2 Subscreen Enable", bg2.regs.sub_enable);
  item("BG3 Subscreen Enable", bg3.regs.sub_enable);
  item("BG4 Subscreen Enable", bg4.regs.sub_enable);
  item("OAM Subscreen Enable", oam.regs.sub_enable);

  //$212e
  item("$212e", "");
  item("BG1 Mainscreen Window Enable", window.regs.bg1_main_enable);
  item("BG2 Mainscreen Window Enable", window.regs.bg2_main_enable);
  item("BG3 Mainscreen Window Enable", window.regs.bg3_main_enable);
  item("BG4 Mainscreen Window Enable", window.regs.bg4_main_enable);
  item("OAM Mainscreen Window Enable", window.regs.oam_main_enable);

  //$212f
  item("$212f", "");
  item("BG1 Subscreen Window Enable", window.regs.bg1_sub_enable);
  item("BG2 Subscreen Window Enable", window.regs.bg2_sub_enable);
  item("BG3 Subscreen Window Enable", window.regs.bg3_sub_enable);
  item("BG4 Subscreen Window Enable", window.regs.bg4_sub_enable);
  item("OAM Subscreen Window Enable", window.regs.oam_sub_enable);

  static char color_window_mask_mode[4][32] = { "Always", "Inside Window Only", "Outside Window Only", "Never" };

  //$2130
  item("$2130", "");
  item("Color Mainscreen Window Mask", color_window_mask_mode[window.regs.col_main_mask & 3]);
  item("Color Subscreen Window Mask", color_window_mask_mode[window.regs.col_sub_mask & 3]);
  item("Color Add/Subtract Mode", !screen.regs.addsub_mode ? "Fixed Color" : "Subscreen");
  item("Direct Color", screen.regs.direct_color);

  //$2131
  item("$2131", "");
  item("Color Mode", !screen.regs.color_mode ? "Add" : "Subtract");
  item("Color Halve", screen.regs.color_halve);
  item("BG1 Color Enable", screen.regs.bg1_color_enable);
  item("BG2 Color Enable", screen.regs.bg2_color_enable);
  item("BG3 Color Enable", screen.regs.bg3_color_enable);
  item("BG4 Color Enable", screen.regs.bg4_color_enable);
  item("OAM Color Enable", screen.regs.oam_color_enable);
  item("Back Color Enable", screen.regs.back_color_enable);

  //$2132
  item("$2132", "");
  item("Color Constant - Blue", ((unsigned)screen.regs.color >> 10) & 0x1f);
  item("Color Constant - Green", ((unsigned)screen.regs.color >> 5) & 0x1f);
  item("Color Constant - Red", ((unsigned)screen.regs.color >> 0) & 0x1f);

  //$2133
  item("$2133", "");
  item("Mode 7 EXTBG", regs.mode7_extbg);
  item("Pseudo Hires", regs.pseudo_hires);
  item("Overscan", regs.overscan);
  item("OAM Interlace", oam.regs.interlace);
  item("Interlace", regs.interlace);

  //$2134-$2136
  item("$2134-$2136", "");
  item("Multiplication Result", string("0x", hex<6>(((int16)regs.m7a * (int8)(regs.m7b >> 8)))));

  //$213c
  item("$213c", "");
  item("H-counter", (unsigned)regs.hcounter);

  //$213d
  item("$213d", "");
  item("V-counter", (unsigned)regs.vcounter);

  //$213e
  item("$213e", "");
  item("Range Over", oam.regs.range_over);
  item("Time Over", oam.regs.time_over);
  item("S-PPU1 Version", (unsigned)ppu1_version);

  //$213f
  item("$213f", "");
  item("Field", cpu.field());
  item("Counters Latched", regs.counters_latched);
  item("Region", system.region() == System::Region::NTSC ? "NTSC" : "PAL");
  item("S-PPU2 Version", (unsigned)ppu2_version);

  #undef item
  return false;
}

#endif
