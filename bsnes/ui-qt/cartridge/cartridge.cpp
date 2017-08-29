#include "../ui-base.hpp"
#include <nall/bps/patch.hpp>
Cartridge cartridge;

//================
//public functions
//================

bool Cartridge::information(const char *filename, Cartridge::Information &info) {
  file fp;
  if(fp.open(filename, file::mode::read) == false) return false;

  if(striend(filename, ".sfc")) {
    if(fp.size() < 0x8000) return false;

    unsigned offset = 0;
    if((fp.size() & 0x7fff) == 512) offset = 512;

    uint16_t complement, checksum;

    fp.seek(0x7fdc + offset);
    complement = fp.readl(2);
    checksum = fp.readl(2);

    unsigned header = offset + (complement + checksum == 65535 ? 0x7fb0 : 0xffb0);

    fp.seek(header + 0x10);
    char name[22];
    fp.read((uint8_t*)name, 21);
    name[21] = 0;
    info.name = decodeJISX0201(name);

    fp.seek(header + 0x29);
    uint8_t region = fp.read();
    info.region = (region <= 1 || region >= 13) ? "NTSC" : "PAL";

    info.romSize = fp.size() & ~0x7fff;

    fp.seek(header + 0x28);
    uint8_t ramsize = fp.read();
    info.ramSize = (ramsize == 0) ? 0 : (1024 << (ramsize & 7));

    return true;
  } else if(striend(filename, ".gb") || striend(filename, ".gbc")) {
    if(fp.size() < 0x200) return false;

    fp.seek(0x134);
    char name[17];
    fp.read((uint8_t*)name, 16);
    name[16] = 0;
    if(name[15] & 0x80) name[15] = 0;  //strip GBC flag
    info.name = decodeJISX0201(name);

    fp.seek(0x147);
    uint8_t mbctype   =   fp.read();
    /*uint8_t romsize =*/ fp.read();
    uint8_t ramsize   =   fp.read();
    uint8_t region    =   fp.read();

    info.region = (region == 0) ? "Japan" : "World";
    info.romSize = fp.size();

    if(mbctype == 0x06) info.ramSize = 256;
    else switch(ramsize) {
      case 0x00: info.ramSize =  0; break;
      case 0x01: info.ramSize =  2 * 1024; break;
      case 0x02: info.ramSize =  8 * 1024; break;
      case 0x03: info.ramSize = 32 * 1024; break;
      case 0x04:
      case 0x05:
      default:   info.ramSize = 128 * 1024;
    }

    return true;
  } else if(striend(filename, ".st")) {
    if(fp.size() < 0x40) return false;

    fp.seek(0x10);
    char name[15];
    fp.read((uint8_t*)name, 14);
    name[14] = 0;
    info.name = decodeJISX0201(name);
    info.region = "NTSC";
    info.romSize = fp.size();

    fp.seek(0x37);
    info.ramSize = fp.read() * 2048;

    return true;
  }
  return false;  //do not parse compressed images
}

bool Cartridge::saveStatesSupported() {
  if(SNES::cartridge.mode() == SNES::Cartridge::Mode::Bsx) return false;

  if(SNES::cartridge.has_st0018()) return false;
  if(SNES::cartridge.has_serial()) return false;

  return true;
}

bool Cartridge::loadNormal(const char *base) {
  unload();
  if(loadCartridge(baseName = base, cartridge.baseXml, SNES::memory::cartrom) == false) return false;
  SNES::cartridge.basename = nall::basename(baseName);

  SNES::cartridge.load(SNES::Cartridge::Mode::Normal,
    lstring() << cartridge.baseXml);

  loadMemory(baseName, ".srm", SNES::memory::cartram);
  loadMemory(baseName, ".rtc", SNES::memory::cartrtc);

  fileName = baseName;
  name = notdir(nall::basename(baseName));

  application.currentRom = base;

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadBsxSlotted(const char *base, const char *slot) {
  unload();
  if(loadCartridge(baseName = base, cartridge.baseXml, SNES::memory::cartrom) == false) return false;
  if(loadCartridge(slotAName = slot, cartridge.slotAXml, SNES::memory::bsxpack) == false) loadEmptyMemoryPack(SNES::memory::bsxpack);
  SNES::cartridge.basename = nall::basename(baseName);

  SNES::cartridge.load(SNES::Cartridge::Mode::BsxSlotted,
    lstring() << cartridge.baseXml << cartridge.slotAXml);

  loadMemory(baseName, ".srm", SNES::memory::cartram);
  loadMemory(baseName, ".rtc", SNES::memory::cartrtc);

  fileName = baseName;
  name = notdir(nall::basename(baseName));
  if(*slot) name << " + " << notdir(nall::basename(slotAName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadBsx(const char *base, const char *slot) {
  unload();
  if(loadCartridge(baseName = base, cartridge.baseXml, SNES::memory::cartrom) == false) return false;
  if(loadCartridge(slotAName = slot, cartridge.slotAXml, SNES::memory::bsxpack) == false) loadEmptyMemoryPack(SNES::memory::bsxpack);
  SNES::cartridge.basename = nall::basename(baseName);

  SNES::cartridge.load(SNES::Cartridge::Mode::Bsx,
    lstring() << cartridge.baseXml << cartridge.slotAXml);

  loadMemory(baseName, ".srm", SNES::memory::cartram);
  loadMemory(baseName, ".psr", SNES::memory::bsxpram);

  if (slotAName != "")
    fileName = slotAName;
  else
    fileName = baseName;
  
  name = *slot
  ? notdir(nall::basename(slotAName))
  : notdir(nall::basename(baseName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadSufamiTurbo(const char *base, const char *slotA, const char *slotB) {
  unload();
  if(loadCartridge(baseName = base, cartridge.baseXml, SNES::memory::cartrom) == false) return false;
  loadCartridge(slotAName = slotA, cartridge.slotAXml, SNES::memory::stArom);
  loadCartridge(slotBName = slotB, cartridge.slotBXml, SNES::memory::stBrom);
  SNES::cartridge.basename = nall::basename(baseName);

  SNES::cartridge.load(SNES::Cartridge::Mode::SufamiTurbo,
    lstring() << cartridge.baseXml << cartridge.slotAXml << cartridge.slotBXml);

  loadMemory(slotAName, ".srm", SNES::memory::stAram);
  loadMemory(slotBName, ".srm", SNES::memory::stBram);

  fileName = slotAName;
  if(!*slotA && !*slotB) name = notdir(nall::basename(baseName));
  else if(!*slotB) name = notdir(nall::basename(slotAName));
  else if(!*slotA) name = notdir(nall::basename(slotBName));
  else name = notdir(nall::basename(slotAName)) << " + " << notdir(nall::basename(slotBName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadSuperGameBoy(const char *base, const char *slot) {
  unload();
  if(loadCartridge(baseName = base, cartridge.baseXml, SNES::memory::cartrom) == false) return false;
  loadCartridge(slotAName = slot, cartridge.slotAXml, SNES::memory::gbrom);
  SNES::cartridge.basename = nall::basename(baseName);

  SNES::cartridge.load(SNES::Cartridge::Mode::SuperGameBoy,
    lstring() << cartridge.baseXml << cartridge.slotAXml);

  loadMemory(slotAName, ".sav", SNES::memory::gbram);
  loadMemory(slotBName, ".rtc", SNES::memory::gbrtc);

  fileName = slotAName;
  name = *slot
  ? notdir(nall::basename(slotAName))
  : notdir(nall::basename(baseName));

  utility.modifySystemState(Utility::LoadCartridge);
  return true;
}

bool Cartridge::loadSpc(const char *base) {
  unload();
  if (!music.opened()) return false;
  
  bool status = false;
  // APU RAM + DSP registers
  uint8_t *dump = new uint8_t[0x10000 + 128];
  // SMP registers
  uint16_t pc;
  uint8_t regs[4];
  uint8_t p;
  audio.clear();
  
  status = music.load_spc(baseName = base, dump, pc, regs, p);
  
  if (status) {
    SNES::cartridge.basename = nall::basename(baseName);

    fileName = baseName;
    name = notdir(nall::basename(baseName));

    application.currentRom = base;
  
    // dummy cartridge
    const char *dummyCart = "\
    <?xml version='1.0' encoding='UTF-8'?>\
    <cartridge region='NTSC' />\
    ";
    SNES::cartridge.load(SNES::Cartridge::Mode::Normal, lstring() << dummyCart);
  
    utility.modifySystemState(Utility::LoadCartridge);
  
    // load the SPC dump
    SNES::smp.load_dump(dump, pc, regs, p);
  
    // put the CPU to sleep (hacky)
    SNES::memory::wram[0] = 0xDB; // STP
    SNES::cpu.regs.pc = 0;
  }
  
  delete[] dump;
  return status;
}

bool Cartridge::loadSnsf(const char *base) {
  unload();
  if (!music.opened()) return false;
  
  uint8_t *data;
  unsigned size;
  audio.clear();
  
  bool status = music.load_snsf(baseName = base, data, size);
  
  if (status) {
    SNES::memory::cartrom.copy(data, size);
    cartridge.baseXml = SNESCartridge(data, size).xmlMemoryMap;
    
    SNES::cartridge.basename = nall::basename(baseName);
    SNES::cartridge.load(SNES::Cartridge::Mode::Normal, lstring() << cartridge.baseXml);

    fileName = baseName;
    name = notdir(nall::basename(baseName));

    application.currentRom = base;

    utility.modifySystemState(Utility::LoadCartridge);
  
    delete[] data;
  }
  
  return status;
}

void Cartridge::saveMemory() {
  if(SNES::cartridge.loaded() == false) return;

  switch(SNES::cartridge.mode()) {
    case SNES::Cartridge::Mode::Normal:
    case SNES::Cartridge::Mode::BsxSlotted: {
      saveMemory(baseName, ".srm", SNES::memory::cartram);
      saveMemory(baseName, ".rtc", SNES::memory::cartrtc);
    } break;

    case SNES::Cartridge::Mode::Bsx: {
      saveMemory(baseName, ".srm", SNES::memory::cartram);
      saveMemory(baseName, ".psr", SNES::memory::bsxpram);
    } break;

    case SNES::Cartridge::Mode::SufamiTurbo: {
      saveMemory(slotAName, ".srm", SNES::memory::stAram);
      saveMemory(slotBName, ".srm", SNES::memory::stBram);
    } break;

    case SNES::Cartridge::Mode::SuperGameBoy: {
      saveMemory(slotAName, ".sav", SNES::memory::gbram);
      saveMemory(slotAName, ".rtc", SNES::memory::gbrtc);
    } break;
  }
}

void Cartridge::saveMemoryPack() {
  if(SNES::cartridge.loaded() == false) return;
  if(SNES::cartridge.has_bsx_slot() == false) return;

  string filename = nall::basename(cartridge.fileName);
  string fullpath = config().path.save;
  
  time_t systemTime = time(0);
  tm *currentTime = localtime(&systemTime);
  char t[512];
  sprintf(t, "%.4u%.2u%.2u-%.2u%.2u%.2u",
    1900 + currentTime->tm_year, 1 + currentTime->tm_mon, currentTime->tm_mday,
    currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec
  );
  filename << "-" << t << ".bs";
  fullpath << filename;

  file fp;
  if(fp.open(fullpath, file::mode::write) == false)
  {
    utility.showMessage(string("Memory Pack save failed."));
    return;
  }

  fp.write(SNES::memory::bsxpack.data(), SNES::memory::bsxpack.size());
  fp.close();
  utility.showMessage(string(filename, " saved."));
}

void Cartridge::unload() {
  if(SNES::cartridge.loaded() == false) return;
  utility.modifySystemState(Utility::UnloadCartridge);
}

void Cartridge::loadCheats() {
  string name;
  name << filepath(nall::basename(baseName), config().path.cheat);
  name << ".cht";
  cheatEditorWindow->load(name);
}

void Cartridge::saveCheats() {
  string name;
  name << filepath(nall::basename(baseName), config().path.cheat);
  name << ".cht";
  cheatEditorWindow->save(name);
}

//=================
//private functions
//=================

bool Cartridge::applyBPS(string &filename, uint8_t *&data, unsigned &size) {
  if(file::exists(filename) == false)
    return false;

  bpspatch bps;
  bps.modify(filename);

  unsigned targetSize = bps.size();
  uint8_t *targetData = new uint8_t[targetSize];

  bps.source(data, size);
  bps.target(targetData, targetSize);

  if(bps.apply() == bpspatch::result::success) {
    delete[] data;
    data = targetData;
    size = targetSize;
    return true;
  } else {
    delete[] targetData;
  }

  return false;
}

bool Cartridge::applyUPS(string &filename, uint8_t *&data, unsigned &size) {
  filemap fp;
  if(fp.open(filename, filemap::mode::read)) {
    uint8_t *outdata = 0;
    unsigned outsize = 0;
    const uint8_t *patchdata = fp.data();
    unsigned patchsize = fp.size();
    ups patcher;
    if(patcher.apply(patchdata, patchsize, data, size, 0, outsize) == ups::result::target_too_small) {
      outdata = new uint8_t[outsize];
      if(patcher.apply(patchdata, patchsize, data, size, outdata, outsize) == ups::result::success) {
        delete[] data;
        data = outdata;
        size = outsize;
        return true;
      } else {
        delete[] outdata;
      }
    }
  }

  return false;
}

bool Cartridge::applyIPS(string &filename, uint8_t *&data, unsigned &size) {
  file fp;
  if(fp.open(filename, file::mode::read) == false) return false;

  unsigned psize = fp.size();
  uint8_t *pdata = new uint8_t[psize];
  fp.read(pdata, psize);
  fp.close();

  if(psize < 8 || pdata[0] != 'P' || pdata[1] != 'A' || pdata[2] != 'T' || pdata[3] != 'C' || pdata[4] != 'H') { delete[] pdata; return false; }

  unsigned outsize = 0;
  uint8_t *outdata = new uint8_t[16 * 1024 * 1024];
  memset(outdata, 0, 16 * 1024 * 1024);
  memcpy(outdata, data, size);

  unsigned offset = 5;
  while(offset < psize - 3) {
    unsigned addr;
    addr  = pdata[offset++] << 16;
    addr |= pdata[offset++] <<  8;
    addr |= pdata[offset++] <<  0;

    unsigned size;
    size  = pdata[offset++] << 8;
    size |= pdata[offset++] << 0;

    if(size == 0) {
      //RLE
      size  = pdata[offset++] << 8;
      size |= pdata[offset++] << 0;

      for(unsigned n = addr; n < addr + size;) {
        outdata[n++] = pdata[offset];
        if(n > outsize) outsize = n;
      }
      offset++;
    } else {
      //uncompressed
      for(unsigned n = addr; n < addr + size;) {
        outdata[n++] = pdata[offset++];
        if(n > outsize) outsize = n;
      }
    }
  }

  delete[] pdata;
  delete[] data;
  data = outdata;
  size = max(size, outsize);
  
  return true;
}

bool Cartridge::loadCartridge(string &filename, string &xml, SNES::MappedRAM &memory) {
  if(file::exists(filename) == false) return false;

  uint8_t *data;
  unsigned size;
  audio.clear();
  if(reader.load(filename, data, size) == false) return false;

  patchApplied = "";

  string bpsName(filepath(nall::basename(filename), config().path.patch), ".bps");
  string upsName(filepath(nall::basename(filename), config().path.patch), ".ups");
  string ipsName(filepath(nall::basename(filename), config().path.patch), ".ips");

  if(config().file.applyPatches) {
    if(applyBPS(bpsName, data, size)) {
      patchApplied = "BPS";
    } else if(applyUPS(upsName, data, size)) {
      patchApplied = "UPS";
    } else if(applyIPS(ipsName, data, size)) {
      patchApplied = "IPS";
    }
  }

  //remove copier header, if it exists
  if((size & 0x7fff) == 512) memmove(data, data + 512, size -= 512);
  
  name = string(nall::basename(filename), ".xml");
  if(patchApplied == "" && file::exists(name)) {
    //prefer manually created XML cartridge mapping
    xml.readfile(name);
  } else {
    //generate XML mapping from data via heuristics
    xml = SNESCartridge(data, size).xmlMemoryMap;
  }

  memory.copy(data, size);
  delete[] data;
  return true;
}

bool Cartridge::loadMemory(const char *filename, const char *extension, SNES::MappedRAM &memory) {
  if(memory.size() == 0) return false;

  string name;
  name << filepath(nall::basename(filename), config().path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode::read) == false) return false;

  unsigned size = fp.size();
  uint8_t *data = new uint8_t[size];
  fp.read(data, size);
  fp.close();

  memory.copy(data, size);
  delete[] data;
  return true;
}

bool Cartridge::saveMemory(const char *filename, const char *extension, SNES::MappedRAM &memory) {
  if(memory.size() == 0) return false;

  string name;
  name << filepath(nall::basename(filename), config().path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode::write) == false) return false;

  fp.write(memory.data(), memory.size());
  fp.close();
  return true;
}

bool Cartridge::loadEmptyMemoryPack(SNES::MappedRAM &memory) {
  uint8_t *emptydata = new uint8_t[0x100000];
  memset(emptydata, 0xFF, 0x100000);
  memory.copy(emptydata, 0x100000);
  delete[] emptydata;
  return true;
}

string Cartridge::decodeJISX0201(const char *text) {
  unsigned length = strlen(text), offset = 0;
  string output;

  for(unsigned i = 0; i < length;) {
    unsigned code = 0;
    uint8_t n = text[i++];

    if(n == 0x00) {
      //string terminator
      break;
    } else if(n >= 0x20 && n <= 0x7f) {
      //ASCII
      code = n;
    } else if(n >= 0xa0 && n <= 0xdf) {
      //ShiftJIS half-width katakana
      unsigned dakuten = 0, handakuten = 0;

      switch(n) {
        case 0xa1: code = 0xe38082; break;  //(period)
        case 0xa2: code = 0xe3808c; break;  //(open quote)
        case 0xa3: code = 0xe3808d; break;  //(close quote)
        case 0xa4: code = 0xe38081; break;  //(comma)
        case 0xa5: code = 0xe383bb; break;  //(separator)
        case 0xa6: code = 0xe383b2; break;  //wo
        case 0xa7: code = 0xe382a1; break;  //la
        case 0xa8: code = 0xe382a3; break;  //li
        case 0xa9: code = 0xe382a5; break;  //lu
        case 0xaa: code = 0xe382a7; break;  //le
        case 0xab: code = 0xe382a9; break;  //lo
        case 0xac: code = 0xe383a3; break;  //lya
        case 0xad: code = 0xe383a5; break;  //lyu
        case 0xae: code = 0xe383a7; break;  //lyo
        case 0xaf: code = 0xe38383; break;  //ltsu
        case 0xb0: code = 0xe383bc; break;  //-
        case 0xb1: code = 0xe382a2; break;  //a
        case 0xb2: code = 0xe382a4; break;  //i
        case 0xb3: code = 0xe382a6; dakuten = 0xe383b4; break;  //u,   vu
        case 0xb4: code = 0xe382a8; break;  //e
        case 0xb5: code = 0xe382aa; break;  //o
        case 0xb6: code = 0xe382ab; dakuten = 0xe382ac; break;  //ka,  ga
        case 0xb7: code = 0xe382ad; dakuten = 0xe382ae; break;  //ki,  gi
        case 0xb8: code = 0xe382af; dakuten = 0xe382b0; break;  //ku,  gu
        case 0xb9: code = 0xe382b1; dakuten = 0xe382b2; break;  //ke,  ge
        case 0xba: code = 0xe382b3; dakuten = 0xe382b4; break;  //ko,  go
        case 0xbb: code = 0xe382b5; dakuten = 0xe382b6; break;  //sa,  za
        case 0xbc: code = 0xe382b7; dakuten = 0xe382b8; break;  //shi, zi
        case 0xbd: code = 0xe382b9; dakuten = 0xe382ba; break;  //su,  zu
        case 0xbe: code = 0xe382bb; dakuten = 0xe382bc; break;  //se,  ze
        case 0xbf: code = 0xe382bd; dakuten = 0xe382be; break;  //so,  zo
        case 0xc0: code = 0xe382bf; dakuten = 0xe38380; break;  //ta,  da
        case 0xc1: code = 0xe38381; dakuten = 0xe38382; break;  //chi, di
        case 0xc2: code = 0xe38384; dakuten = 0xe38385; break;  //tsu, du
        case 0xc3: code = 0xe38386; dakuten = 0xe38387; break;  //te,  de
        case 0xc4: code = 0xe38388; dakuten = 0xe38389; break;  //to,  do
        case 0xc5: code = 0xe3838a; break;  //na
        case 0xc6: code = 0xe3838b; break;  //ni
        case 0xc7: code = 0xe3838c; break;  //nu
        case 0xc8: code = 0xe3838d; break;  //ne
        case 0xc9: code = 0xe3838e; break;  //no
        case 0xca: code = 0xe3838f; dakuten = 0xe38390; handakuten = 0xe38391; break;  //ha, ba, pa
        case 0xcb: code = 0xe38392; dakuten = 0xe38393; handakuten = 0xe38394; break;  //hi, bi, pi
        case 0xcc: code = 0xe38395; dakuten = 0xe38396; handakuten = 0xe38397; break;  //fu, bu, pu
        case 0xcd: code = 0xe38398; dakuten = 0xe38399; handakuten = 0xe3839a; break;  //he, be, pe
        case 0xce: code = 0xe3839b; dakuten = 0xe3839c; handakuten = 0xe3839d; break;  //ho, bo, po
        case 0xcf: code = 0xe3839e; break;  //ma
        case 0xd0: code = 0xe3839f; break;  //mi
        case 0xd1: code = 0xe383a0; break;  //mu
        case 0xd2: code = 0xe383a1; break;  //me
        case 0xd3: code = 0xe383a2; break;  //mo
        case 0xd4: code = 0xe383a4; break;  //ya
        case 0xd5: code = 0xe383a6; break;  //yu
        case 0xd6: code = 0xe383a8; break;  //yo
        case 0xd7: code = 0xe383a9; break;  //ra
        case 0xd8: code = 0xe383aa; break;  //ri
        case 0xd9: code = 0xe383ab; break;  //ru
        case 0xda: code = 0xe383ac; break;  //re
        case 0xdb: code = 0xe383ad; break;  //ro
        case 0xdc: code = 0xe383af; break;  //wa
        case 0xdd: code = 0xe383b3; break;  //n
      }

      if(dakuten && ((uint8_t)text[i] == 0xde)) {
        code = dakuten;
        i++;
      } else if(handakuten && ((uint8_t)text[i] == 0xdf)) {
        code = handakuten;
        i++;
      }
    }

    if(code) {
      if((uint8_t)(code >> 16)) output[offset++] = (char)(code >> 16);
      if((uint8_t)(code >>  8)) output[offset++] = (char)(code >>  8);
      if((uint8_t)(code >>  0)) output[offset++] = (char)(code >>  0);
    }
  }

  output[offset] = 0;
  return output;
}
