class PPUDebugger : public PPU, public ChipDebugger {
public:
  uint8 vram_mmio_read(uint16 addr);
  void vram_mmio_write(uint16 addr, uint8 data);

  uint8 oam_mmio_read(uint16 addr);
  void oam_mmio_write(uint16 addr, uint8 data);

  uint8 cgram_mmio_read(uint16 addr);
  void cgram_mmio_write(uint16 addr, uint8 data);

  uint16 bg_screen_addr(unsigned index) const;
  uint16 bg_tile_addr(unsigned index) const;
  uint16 oam_tile_addr(unsigned index) const;
  uint8 oam_base_size() const;
  bool property(unsigned id, string &name, string &value);
};
