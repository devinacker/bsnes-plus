#include <snes.hpp>

#define MEMORY_CPP
namespace SNES {

Bus bus;

#include "serialization.cpp"

namespace memory {
  MMIOAccess mmio;
  StaticRAM wram(128 * 1024);
  StaticRAM apuram(64 * 1024);
  VRAM vram;
  StaticRAM oam(544);
  StaticRAM cgram(512);

  UnmappedMemory memory_unmapped;
  UnmappedMMIO mmio_unmapped;
};

unsigned UnmappedMemory::size() const { return 16 * 1024 * 1024; }
uint8 UnmappedMemory::read(unsigned) { return cpu.regs.mdr; }
void UnmappedMemory::write(unsigned, uint8) {}

uint8 UnmappedMMIO::mmio_read(unsigned) { return cpu.regs.mdr; }
void UnmappedMMIO::mmio_write(unsigned, uint8) {}

MMIO* MMIOAccess::handle(unsigned addr) {
  return mmio[addr];
}

void MMIOAccess::map(unsigned addr, MMIO &access) {
  assert(addr >= Min && addr <= Max);
  mmio[addr] = &access;
}

void MMIOAccess::map(unsigned addr_lo, unsigned addr_hi, MMIO &access) {
  assert(addr_lo <= addr_hi);
  assert(addr_lo >= Min && addr_lo <= Max);
  assert(addr_hi >= Min && addr_hi <= Max);
  for(unsigned addr = addr_lo; addr <= addr_hi; addr++) {
    mmio[addr] = &access;
  }
}

unsigned MMIOAccess::size() const { return 0x8000; }

uint8 MMIOAccess::read(unsigned addr) {
  return mmio[addr]->mmio_read(addr);
}

void MMIOAccess::write(unsigned addr, uint8 data) {
  if(!debugger_access()) mmio[addr]->mmio_write(addr, data);
}

MMIOAccess::MMIOAccess() {
  for(unsigned i = 0; i < 0x8000; i++) mmio[i] = &memory::mmio_unmapped;
}

unsigned Bus::mirror(unsigned addr, unsigned size) {
  unsigned base = 0;
  if(size) {
    unsigned mask = 1 << 23;
    while(addr >= size) {
      while(!(addr & mask)) mask >>= 1;
      addr -= mask;
      if(size > mask) {
        size -= mask;
        base += mask;
      }
      mask >>= 1;
    }
    base += addr;
  }
  return base;
}

void Bus::map(unsigned addr, Memory &access, unsigned offset) {
  Page &p = page[addr >> 8];
  p.access = &access;
  p.offset = offset - addr;
}

void Bus::map(
  MapMode mode,
  uint8  bank_lo, uint8  bank_hi,
  uint16 addr_lo, uint16 addr_hi,
  Memory &access, unsigned offset, unsigned size
) {
  assert(bank_lo <= bank_hi);
  assert(addr_lo <= addr_hi);

  uint8 page_lo = addr_lo >> 8;
  uint8 page_hi = addr_hi >> 8;
  unsigned index = 0;

  switch(mode) {
    case MapMode::Direct: {
      for(unsigned bank = bank_lo; bank <= bank_hi; bank++) {
        for(unsigned page = page_lo; page <= page_hi; page++) {
          map((bank << 16) + (page << 8), access, (bank << 16) + (page << 8));
        }
      }
    } break;

    case MapMode::Linear: {
      for(unsigned bank = bank_lo; bank <= bank_hi; bank++) {
        for(unsigned page = page_lo; page <= page_hi; page++) {
          map((bank << 16) + (page << 8), access, mirror(offset + index, access.size()));
          index += 256;
          if(size) index %= size;
        }
      }
    } break;

    case MapMode::Shadow: {
      for(unsigned bank = bank_lo; bank <= bank_hi; bank++) {
        index += page_lo * 256;
        if(size) index %= size;

        for(unsigned page = page_lo; page <= page_hi; page++) {
          map((bank << 16) + (page << 8), access, mirror(offset + index, access.size()));
          index += 256;
          if(size) index %= size;
        }

        index += (255 - page_hi) * 256;
        if(size) index %= size;
      }
    } break;
  }
}

bool Bus::load_cart() {
  if(cartridge.loaded() == true) return false;

  map_reset();
  map_xml();
  map_system();
  return true;
}

void Bus::unload_cart() {
}

void Bus::map_reset() {
  map(MapMode::Direct, 0x00, 0xff, 0x0000, 0xffff, memory::memory_unmapped);
  map(MapMode::Shadow, 0x00, 0x3f, MMIOAccess::Min, MMIOAccess::Max, memory::mmio);
  map(MapMode::Shadow, 0x80, 0xbf, MMIOAccess::Min, MMIOAccess::Max, memory::mmio);
  memory::mmio.map(MMIOAccess::Min, MMIOAccess::Max, memory::mmio_unmapped);
}

void Bus::map_xml() {
  foreach(m, cartridge.mapping) {
    if(m.memory && (memory::cartram.size() >= m.min_ram_size)) {
      map(m.mode, m.banklo, m.bankhi, m.addrlo, m.addrhi, *m.memory, m.offset, m.size);
    } else if(m.mmio) {
      memory::mmio.map(m.addrlo, m.addrhi, *m.mmio);
    }
  }
}

void Bus::map_system() {
  map(MapMode::Linear, 0x00, 0x3f, 0x0000, 0x1fff, memory::wram, 0x000000, 0x002000);
  map(MapMode::Linear, 0x80, 0xbf, 0x0000, 0x1fff, memory::wram, 0x000000, 0x002000);
  map(MapMode::Linear, 0x7e, 0x7f, 0x0000, 0xffff, memory::wram);
  
  unsigned vram_size = 1<<16;
  if (SNES::PPU::SupportsVRAMExpansion) {
    vram_size = 1<<(16+max(0,min(2,config().vram_size)));
  }
  memory::vram.map(new uint8[vram_size], vram_size);
}

void Bus::power() {
  foreach(n, memory::wram) n = random(config().cpu.wram_init_value);
}

void Bus::reset() {
}

}
