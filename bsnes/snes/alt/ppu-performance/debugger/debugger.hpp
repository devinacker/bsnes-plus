class PPUDebugger : public PPU, public ChipDebugger {
public:
  uint8 vram_read(unsigned addr);
  void vram_write(unsigned addr, uint8 data);

  uint8 oam_read(unsigned addr);
  void oam_write(unsigned addr, uint8 data);

  uint8 cgram_read(unsigned addr);
  void cgram_write(unsigned addr, uint8 data);

  bool property(unsigned id, string &name, string &value);
};
