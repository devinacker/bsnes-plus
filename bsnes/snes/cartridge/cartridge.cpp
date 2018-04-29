#include <snes.hpp>

#include <nall/crc32.hpp>
#include <nall/sha256.hpp>

#define CARTRIDGE_CPP
namespace SNES {

#include "xml.cpp"
#include "serialization.cpp"

namespace memory {
  MappedRAM cartrom, cartram, cartrtc;
  MappedRAM bsxpack, bsxpram;
  MappedRAM stArom, stAram;
  MappedRAM stBrom, stBram;
  MappedRAM gbrom, gbram, gbrtc;
};

Cartridge cartridge;

int Cartridge::rom_offset(unsigned addr) const {
  Bus::Page &page = bus.page[addr >> 8];
  if (page.access == &memory::cartrom ||
      page.access == &memory::cx4rom ||
      page.access == &memory::gsurom ||
      page.access == &memory::fxrom ||
      page.access == &memory::vsprom) {
    return page.offset + addr;
  }
  
  return -1;
}

void Cartridge::load(Mode cartridge_mode, const lstring &xml_list) {
  mode = cartridge_mode;
  region = Region::NTSC;
  ram_size = 0;
  spc7110_data_rom_offset = 0x100000;
  st_A_ram_size = 0;
  st_B_ram_size = 0;
  bsxpack_type = BSXPackType::FlashROM;
  supergameboy_version = SuperGameBoyVersion::Version1;
  supergameboy_ram_size = 0;
  supergameboy_rtc_size = 0;

  has_bsx_slot   = false;
  has_superfx    = false;
  has_sa1        = false;
  has_necdsp     = false;
  has_srtc       = false;
  has_sdd1       = false;
  has_spc7110    = false;
  has_spc7110rtc = false;
  has_cx4        = false;
  has_obc1       = false;
  has_st0018     = false;
  has_msu1       = false;
  has_serial     = false;

  parse_xml(xml_list);
//print(xml_list[0], "\n\n");

  // autodetect MSU1 if it wasn't specified in a manifest
  if(!has_msu1 && file::exists(string(basename(), ".msu"))) {
    has_msu1 = true;
    
    Mapping m(msu1);
    m.addrlo = 0x2000;
    m.addrhi = 0x2007;
    mapping.append(m);
  }

  if(ram_size > 0) {
    memory::cartram.map(allocate<uint8_t>(ram_size, 0xff), ram_size);
  }

  if(has_srtc || has_spc7110rtc) {
    memory::cartrtc.map(allocate<uint8_t>(20, 0xff), 20);
  }

  if(mode == Mode::Bsx) {
    memory::bsxpram.map(allocate<uint8_t>(512 * 1024, 0xff), 512 * 1024);
  }

  if(mode == Mode::SufamiTurbo) {
    if(st_A_ram_size) memory::stAram.map(allocate<uint8_t>(st_A_ram_size, 0xff), st_A_ram_size);
    if(st_B_ram_size) memory::stBram.map(allocate<uint8_t>(st_B_ram_size, 0xff), st_B_ram_size);
  }

  if(mode == Mode::SuperGameBoy) {
    if(memory::gbrom.data()) {
      if(supergameboy_ram_size) memory::gbram.map(allocate<uint8_t>(supergameboy_ram_size, 0xff), supergameboy_ram_size);
      if(supergameboy_rtc_size) memory::gbrtc.map(allocate<uint8_t>(supergameboy_rtc_size, 0x00), supergameboy_rtc_size);
    }
  }

  memory::cartrom.write_protect(true);
  memory::cartram.write_protect(false);
  memory::cartrtc.write_protect(false);
  memory::bsxpack.write_protect(true);
  memory::bsxpram.write_protect(false);
  memory::stArom.write_protect(true);
  memory::stAram.write_protect(false);
  memory::stBrom.write_protect(true);
  memory::stBram.write_protect(false);
  memory::gbrom.write_protect(true);
  memory::gbram.write_protect(false);
  memory::gbrtc.write_protect(false);

  unsigned checksum = ~0;         foreach(n, memory::cartrom) checksum = crc32_adjust(checksum, n);
  if(memory::bsxpack.size() != 0) foreach(n, memory::bsxpack) checksum = crc32_adjust(checksum, n);
  if(memory::stArom.size()  != 0) foreach(n, memory::stArom ) checksum = crc32_adjust(checksum, n);
  if(memory::stBrom.size()  != 0) foreach(n, memory::stBrom ) checksum = crc32_adjust(checksum, n);
  if(memory::gbrom.size()   != 0) foreach(n, memory::gbrom  ) checksum = crc32_adjust(checksum, n);
  crc32 = ~checksum;

  sha256_ctx sha;
  uint8_t shahash[32];
  sha256_init(&sha);
  sha256_chunk(&sha, memory::cartrom.data(), memory::cartrom.size());
  sha256_final(&sha);
  sha256_hash(&sha, shahash);

  string hash;
  foreach(n, shahash) hash << hex<2>(n);
  sha256 = hash;

  bus.load_cart();
  system.serialize_init();
  loaded = true;
}

void Cartridge::unload() {
  memory::cartrom.reset();
  memory::cartram.reset();
  memory::cartrtc.reset();
  memory::bsxpack.reset();
  memory::bsxpram.reset();
  memory::stArom.reset();
  memory::stAram.reset();
  memory::stBrom.reset();
  memory::stBram.reset();
  memory::gbrom.reset();
  memory::gbram.reset();
  memory::gbrtc.reset();

  if(loaded == false) return;
  bus.unload_cart();
  loaded = false;
}

Memory& Cartridge::bsxpack_access() {
  if(memory::bsxpack.size() == 0) return memory::memory_unmapped;
  return (bsxpack_type == BSXPackType::FlashROM) ? (Memory&)bsxflash : (Memory&)memory::bsxpack;
}

Cartridge::Cartridge() {
  loaded = false;
  unload();
}

Cartridge::~Cartridge() {
  unload();
}

}
