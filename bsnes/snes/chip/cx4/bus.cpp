#ifdef CX4_CPP

Cx4Bus cx4bus;

namespace memory {
  UnmappedCx4 cx4_unmapped;
  Cx4ROM cx4rom;
  Cx4RAM cx4ram;
}

void Cx4Bus::init() {
  map(MapMode::Direct, 0x00, 0xff, 0x0000, 0xffff, memory::cx4_unmapped);

  map(MapMode::Linear, 0x00, 0x3f, 0x8000, 0xffff, memory::cartrom);
  map(MapMode::Linear, 0x80, 0xbf, 0x8000, 0xffff, memory::cartrom);
  map(MapMode::Linear, 0x70, 0x77, 0x0000, 0x7fff, memory::cartram);
}

unsigned UnmappedCx4::size() const { return 16 * 1024 * 1024; }
uint8 UnmappedCx4::read(unsigned) { return 0; }
void UnmappedCx4::write(unsigned, uint8) {}

unsigned Cx4ROM::size() const { return memory::cartrom.size(); }
uint8 Cx4ROM::read(unsigned addr) { return cx4.rom_read(addr); }
void Cx4ROM::write(unsigned, uint8) {}

unsigned Cx4RAM::size() const { return memory::cartram.size(); }
uint8 Cx4RAM::read(unsigned addr) { return cx4.ram_read(addr); }
void Cx4RAM::write(unsigned addr, uint8 data) { return cx4.ram_write(addr, data); }

#endif
