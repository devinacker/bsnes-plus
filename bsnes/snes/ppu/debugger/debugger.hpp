class PPUDebugger : public PPU, public ChipDebugger {
public:
  uint8 vram_read(unsigned addr);
  void vram_write(unsigned addr, uint8 data);
  
  uint8 oam_read(unsigned addr);
  void oam_write(unsigned addr, uint8 data);
  
  uint8 cgram_read(unsigned addr);
  void cgram_write(unsigned addr, uint8 data);
  
  uint8  bg_mode() const;
  uint16 bg_screen_addr(unsigned index) const;
  uint8  bg_screen_size(unsigned index) const;
  uint16 bg_tile_addr(unsigned index) const;
  uint8  bg_tile_size(unsigned index) const;
  uint16 oam_tile_addr(unsigned index) const;
  uint8  oam_base_size() const;
  unsigned  oam_first_sprite() const;
  bool mode7_extbg() const;
  bool property(unsigned id, string &name, string &value);
};
