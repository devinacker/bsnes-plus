#ifndef NALL_SNES_CARTRIDGE_HPP
#define NALL_SNES_CARTRIDGE_HPP

namespace nall {

class SNESCartridge {
public:
  string xmlMemoryMap;
  inline SNESCartridge(const uint8_t *data, unsigned size);

//private:
  inline void read_header(const uint8_t *data, unsigned size);
  inline unsigned find_header(const uint8_t *data, unsigned size);
  inline unsigned score_header(const uint8_t *data, unsigned size, unsigned addr);
  inline unsigned gameboy_ram_size(const uint8_t *data, unsigned size);
  inline bool gameboy_has_rtc(const uint8_t *data, unsigned size);
  inline unsigned sufamiturbo_ram_size(const uint8_t *data, unsigned size);

  enum HeaderField {
    CartName    = 0x00,
    Mapper      = 0x15,
    RomType     = 0x16,
    RomSize     = 0x17,
    RamSize     = 0x18,
    CartRegion  = 0x19,
    Company     = 0x1a,
    Version     = 0x1b,
    Complement  = 0x1c,  //inverse checksum
    Checksum    = 0x1e,
    ResetVector = 0x3c,
  };

  enum Type {
    TypeNormal,
    TypeBsxSlotted,
    TypeBsxBios,
    TypeBsx,
    TypeSufamiTurboBios,
    TypeSufamiTurbo,
    TypeSuperGameBoy1Bios,
    TypeSuperGameBoy2Bios,
    TypeGameBoy,
    TypeUnknown,
  };

  enum Region {
    NTSC,
    PAL,
  };

  enum MemoryMapper {
    SGBROM,
    LoROM,
    HiROM,
    ExHiROM,
    SuperFXROM,
    SA1ROM,
    SDD1ROM,
    SPC7110ROM,
    BSCLoROM,
    BSCHiROM,
    BSXROM,
    STROM,
    Cx4ROM,
  };

  enum DSP1MemoryMapper {
    DSP1Unmapped,
    DSP1LoROM1MB,
    DSP1LoROM2MB,
    DSP1HiROM,
  };

  enum BSXPackType {
    FlashROM,
    MaskROM,
  };

  unsigned rom_size;
  unsigned ram_size;

  Type type;
  Region region;
  MemoryMapper mapper;
  DSP1MemoryMapper dsp1_mapper;
  BSXPackType bsxpack_type;

  bool has_bsx_slot;
  bool has_spc7110rtc;
  bool has_srtc;
  bool has_dsp1;
  bool has_dsp2;
  bool has_dsp3;
  bool has_dsp4;
  bool has_obc1;
  bool has_st010;
  bool has_st011;
  bool has_st018;
  bool has_dos;
};

SNESCartridge::SNESCartridge(const uint8_t *data, unsigned size) {
  read_header(data, size);

  string xml = "<?xml version='1.0' encoding='UTF-8'?>\n";

  if(type == TypeBsx) {
    xml << "<cartridge type='";
    if(bsxpack_type == FlashROM) {
      xml << "FlashROM";
    } else {
      xml << "MaskROM";
    }
    xml << "'/>\n";
    xmlMemoryMap = xml;
    return;
  }

  if(type == TypeSufamiTurbo) {
    xml << "<cartridge>";
    if(sufamiturbo_ram_size(data, size) > 0) {
      xml << "  <ram size='" << hex(sufamiturbo_ram_size(data, size)) << "'/>\n";
    }
    xml << "</cartridge>\n";
    xmlMemoryMap = xml;
    return;
  }

  if(type == TypeGameBoy) {
    xml << "<cartridge rtc='" << gameboy_has_rtc(data, size) << "'>\n";
    if(gameboy_ram_size(data, size) > 0) {
      xml << "  <ram size='" << hex(gameboy_ram_size(data, size)) << "'/>\n";
    }
    xml << "</cartridge>\n";
    xmlMemoryMap = xml;
    return;
  }

  xml << "<cartridge region='";
  if(region == NTSC) {
    xml << "NTSC";
  } else {
    xml << "PAL";
  }
  xml << "'>\n";

  if(mapper == SGBROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='linear' address='00-7d:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='40-7d:0000-7fff'/>\n";
    xml << "    <map mode='linear' address='80-ff:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='c0-ff:0000-7fff'/>\n";
    xml << "  </rom>\n";
    const unsigned revision = (type == TypeSuperGameBoy2Bios) ? 2 : 1;
    xml << "  <supergameboy revision='" << revision << "'>\n";
    xml << "    <map address='00-3f:6000-67ff'/>\n";
    xml << "    <map address='00-3f:7000-7fff'/>\n";
    xml << "    <map address='80-bf:6000-67ff'/>\n";
    xml << "    <map address='80-bf:7000-7fff'/>\n";
    xml << "  </supergameboy>\n";
    
  } else if(mapper == LoROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='linear' address='00-7d:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='80-ff:8000-ffff'/>\n";
    xml << "  </rom>\n";

    if(ram_size > 0) {
      xml << "  <ram size='" << hex(ram_size) << "'>\n";
      const unsigned range = (rom_size > 0x200000) || (ram_size > 32 * 1024) ? 0x7fff : 0xffff;
      xml << "    <map mode='linear' address='70-7d:0000-" << hex(range) << "'/>\n";
      xml << "    <map mode='linear' address='f0-ff:0000-" << hex(range) << "'/>\n";
      xml << "  </ram>\n";
    }
    
  } else if(mapper == HiROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='shadow' address='00-3f:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='40-7d:0000-ffff'/>\n";
    xml << "    <map mode='shadow' address='80-bf:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='c0-ff:0000-ffff'/>\n";
    xml << "  </rom>\n";

    if(ram_size > 0) {
      xml << "  <ram size='" << hex(ram_size) << "'>\n";
      xml << "    <map mode='linear' address='20-3f:6000-7fff'/>\n";
      xml << "    <map mode='linear' address='a0-bf:6000-7fff'/>\n";
      xml << "  </ram>\n";
    }
    
  } else if(mapper == ExHiROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='shadow' address='00-3f:8000-ffff' offset='400000'/>\n";
    xml << "    <map mode='linear' address='40-7d:0000-ffff' offset='400000'/>\n";
    xml << "    <map mode='shadow' address='80-bf:8000-ffff' offset='000000'/>\n";
    xml << "    <map mode='linear' address='c0-ff:0000-ffff' offset='000000'/>\n";
    xml << "  </rom>\n";

    if(ram_size > 0) {
      xml << "  <ram size='" << hex(ram_size) << "'>\n";
      xml << "    <map mode='linear' address='20-3f:6000-7fff'/>\n";
      xml << "    <map mode='linear' address='70-7d:0000-7fff'/>\n";
      xml << "    <map mode='linear' address='a0-bf:6000-7fff'/>\n";
      xml << "  </ram>\n";
    }
    
  } else if(mapper == SuperFXROM) {
    xml << "  <superfx revision='2'>\n";
    xml << "    <rom>\n";
    xml << "      <map mode='linear' address='00-3f:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='40-5f:0000-ffff'/>\n";
    xml << "      <map mode='linear' address='80-bf:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='c0-df:0000-ffff'/>\n";
    xml << "    </rom>\n";

    if(ram_size > 0) {
      xml << "    <ram size='" << hex(ram_size) << "'>\n";
      xml << "      <map mode='linear' address='00-3f:6000-7fff' size='2000'/>\n";
      xml << "      <map mode='linear' address='60-7d:0000-ffff'/>\n";
      xml << "      <map mode='linear' address='80-bf:6000-7fff' size='2000'/>\n";
      xml << "      <map mode='linear' address='e0-ff:0000-ffff'/>\n";
      xml << "    </ram>\n";
    }
    xml << "    <mmio>\n";
    xml << "      <map address='00-3f:3000-34ff'/>\n";
    xml << "      <map address='80-bf:3000-34ff'/>\n";
    xml << "    </mmio>\n";
    xml << "  </superfx>\n";
    
  } else if(mapper == SA1ROM) {
    xml << "  <sa1>\n";
    xml << "    <rom>\n";
    xml << "      <map mode='linear' address='00-3f:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='80-bf:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='c0-ff:0000-ffff'/>\n";
    xml << "    </rom>\n";
    xml << "    <iram size='800'>\n";
    xml << "      <map mode='linear' address='00-3f:3000-37ff'/>\n";
    xml << "      <map mode='linear' address='80-bf:3000-37ff'/>\n";
    xml << "    </iram>\n";

    if(ram_size > 0) {
      xml << "    <bwram size='" << hex(ram_size) << "'>\n";
      xml << "      <map mode='linear' address='00-3f:6000-7fff'/>\n";
      xml << "      <map mode='linear' address='40-4f:0000-ffff'/>\n";
      xml << "      <map mode='linear' address='80-bf:6000-7fff'/>\n";
      xml << "    </bwram>\n";
    }
    xml << "    <mmio>\n";
    xml << "      <map address='00-3f:2200-23ff'/>\n";
    xml << "      <map address='80-bf:2200-23ff'/>\n";
    xml << "    </mmio>\n";
    xml << "  </sa1>\n";

    if(has_bsx_slot) {
      xml << "  <bsx/>";  // Super MMC controls BS-X slot mapping
    }
    
  } else if(mapper == SDD1ROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='linear' address='00-3f:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='80-bf:8000-ffff'/>\n";
    xml << "  </rom>\n";

    if(ram_size > 0) {
      xml << "  <ram size='" << hex(ram_size) << "'>\n";
      xml << "    <map mode='linear' address='00-3f:6000-7fff'/>\n";
      xml << "    <map mode='linear' address='80-bf:6000-7fff'/>\n";
      xml << "    <map mode='linear' address='70-73:0000-ffff'/>\n";
      xml << "  </ram>\n";
    }
    xml << "  <sdd1>\n";
    xml << "    <mcu>\n";
    xml << "      <map address='c0-ff:0000-ffff'/>\n";
    xml << "    </mcu>\n";
    xml << "    <mmio>\n";
    xml << "      <map address='00-3f:4800-480f'/>\n";
    xml << "      <map address='80-bf:4800-480f'/>\n";
    xml << "    </mmio>\n";
    xml << "  </sdd1>\n";
    
  } else if(mapper == SPC7110ROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='shadow' address='00-0f:8000-ffff'/>\n";
    xml << "    <map mode='shadow' address='80-bf:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='c0-cf:0000-ffff'/>\n";
    if (size >= 0x700000) {
      // Tengai Makyou Zero english translation
      xml << "    <map mode='linear' address='40-4f:0000-ffff' offset='600000'/>\n";
    }
    xml << "  </rom>\n";

    xml << "  <spc7110>\n";
    xml << "    <mcu>\n";
    xml << "      <map address='d0-ff:0000-ffff' offset='100000' size='" << hex(size - 0x100000) << "'/>\n";
    xml << "    </mcu>\n";

    if(ram_size > 0) {
      xml << "    <ram size='" << hex(ram_size) << "'>\n";
      xml << "      <map mode='linear' address='00-3f:6000-7fff'/>\n";
      xml << "      <map mode='linear' address='80-bf:6000-7fff'/>\n";
      xml << "    </ram>\n";
    }
    xml << "    <mmio>\n";
    xml << "      <map address='00-3f:4800-483f'/>\n";
    xml << "      <map address='80-bf:4800-483f'/>\n";
    xml << "    </mmio>\n";

    if(has_spc7110rtc) {
      xml << "    <rtc>\n";
      xml << "      <map address='00-3f:4840-4842'/>\n";
      xml << "      <map address='80-bf:4840-4842'/>\n";
      xml << "    </rtc>\n";
    }
    xml << "    <dcu>\n";
    xml << "      <map address='50:0000-ffff'/>\n";
    xml << "      <map address='58:0000-ffff'/>\n";
    xml << "    </dcu>\n";
    xml << "  </spc7110>\n";
    
  } else if(mapper == BSCLoROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='linear' address='00-1f:8000-ffff' offset='000000'/>\n";
    xml << "    <map mode='linear' address='20-3f:8000-ffff' offset='100000'/>\n";
    xml << "    <map mode='linear' address='80-9f:8000-ffff' offset='200000'/>\n";
    xml << "    <map mode='linear' address='a0-bf:8000-ffff' offset='100000'/>\n";
    xml << "  </rom>\n";

    if(ram_size > 0) {
      xml << "  <ram size='" << hex(ram_size) << "'>\n";
      xml << "    <map mode='linear' address='70-7d:0000-7fff'/>\n";
      xml << "    <map mode='linear' address='f0-ff:0000-7fff'/>\n";
      xml << "  </ram>\n";
    }
    xml << "  <bsx>\n";
    xml << "    <slot>\n";
    xml << "      <map mode='linear' address='c0-ef:0000-7fff'/>\n";
    xml << "      <map mode='linear' address='c0-ef:8000-ffff'/>\n";
    xml << "    </slot>\n";
    xml << "  </bsx>\n";
    
  } else if(mapper == BSCHiROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='shadow' address='00-1f:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='40-5f:0000-ffff'/>\n";
    xml << "    <map mode='shadow' address='80-9f:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='c0-df:0000-ffff'/>\n";
    xml << "  </rom>\n";

    if(ram_size > 0) {
      xml << "  <ram size='" << hex(ram_size) << "'>\n";
      xml << "    <map mode='linear' address='20-3f:6000-7fff'/>\n";
      xml << "    <map mode='linear' address='a0-bf:6000-7fff'/>\n";
      xml << "  </ram>\n";
    }
    xml << "  <bsx>\n";
    xml << "    <slot>\n";
    xml << "      <map mode='shadow' address='20-3f:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='60-7d:0000-ffff'/>\n";
    xml << "      <map mode='shadow' address='a0-bf:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='e0-ff:0000-ffff'/>\n";
    xml << "    </slot>\n";
    xml << "  </bsx>\n";
    
  } else if(mapper == BSXROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='linear' address='00-3f:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='80-bf:8000-ffff'/>\n";
    xml << "  </rom>\n";
    xml << "  <ram size='8000'>\n";
    xml << "    <map mode='linear' address='10-17:5000-5fff'/>\n";
    xml << "  </ram>\n";
    xml << "  <bsx>\n";
    xml << "    <mcc>\n";
    xml << "      <map address='00-0f:5000-5fff'/>\n";
    xml << "    </mcc>\n";
    xml << "  </bsx>\n";
    
  } else if(mapper == STROM) {
    xml << "  <rom>\n";
    xml << "    <map mode='linear' address='00-1f:8000-ffff'/>\n";
    xml << "    <map mode='linear' address='80-9f:8000-ffff'/>\n";
    xml << "  </rom>\n";
    xml << "  <sufamiturbo>\n";
    xml << "    <slot id='A'>\n";
    xml << "      <rom>\n";
    xml << "        <map mode='linear' address='20-3f:8000-ffff'/>\n";
    xml << "        <map mode='linear' address='a0-bf:8000-ffff'/>\n";
    xml << "      </rom>\n";
    xml << "      <ram>\n";
    xml << "        <map mode='linear' address='60-6f:0000-ffff'/>\n";
    xml << "        <map mode='linear' address='e0-ef:0000-ffff'/>\n";
    xml << "      </ram>\n";
    xml << "    </slot>\n";
    xml << "    <slot id='B'>\n";
    xml << "      <rom>\n";
    xml << "        <map mode='linear' address='40-5f:0000-7fff'/>\n";
    xml << "        <map mode='linear' address='40-5f:8000-ffff'/>\n";
    xml << "        <map mode='linear' address='c0-df:0000-7fff'/>\n";
    xml << "        <map mode='linear' address='c0-df:8000-ffff'/>\n";
    xml << "      </rom>\n";
    xml << "      <ram>\n";
    xml << "        <map mode='linear' address='70-7d:0000-ffff'/>\n";
    xml << "        <map mode='linear' address='f0-ff:0000-ffff'/>\n";
    xml << "      </ram>\n";
    xml << "    </slot>\n";
    xml << "  </sufamiturbo>\n";
    
  } else if(mapper == Cx4ROM) {
    xml << "  <cx4 frequency='20000000'>\n";
    xml << "    <rom>\n";
    xml << "      <map mode='linear' address='00-3f:8000-ffff'/>\n";
    xml << "      <map mode='linear' address='80-bf:8000-ffff'/>\n";
    xml << "    </rom>\n";
    xml << "    <ram size='" << hex(ram_size) << "'>\n";
    xml << "      <map mode='linear' address='70-77:0000-7fff'/>\n";
    xml << "    </ram>\n";
    xml << "    <map address='00-3f:6000-7fff'/>\n";
    xml << "    <map address='80-bf:6000-7fff'/>\n";
    xml << "  </cx4>\n";
  }

  if(has_srtc) {
    xml << "  <srtc>\n";
    xml << "    <map address='00-3f:2800-2801'/>\n";
    xml << "    <map address='80-bf:2800-2801'/>\n";
    xml << "  </srtc>\n";
  }

  if(has_dsp1) {
    xml << "  <necdsp revision='upd7725' frequency='8000000' program='dsp1b.bin' sha256='4d42db0f36faef263d6b93f508e8c1c4ae8fc2605fd35e3390ecc02905cd420c'>\n";
    if(dsp1_mapper == DSP1LoROM1MB) {
      xml << "    <dr mask='004000' test='000000'/>\n";
      xml << "    <sr mask='004000' test='004000'/>\n";
      xml << "    <map address='20-3f:8000-ffff'/>\n";
      xml << "    <map address='a0-bf:8000-ffff'/>\n";
    } else if(dsp1_mapper == DSP1LoROM2MB) {
      xml << "    <dr mask='004000' test='000000'/>\n";
      xml << "    <sr mask='004000' test='004000'/>\n";
      xml << "    <map address='60-6f:0000-7fff'/>\n";
      xml << "    <map address='e0-ef:0000-7fff'/>\n";
    } else if(dsp1_mapper == DSP1HiROM) {
      xml << "    <dr mask='001000' test='000000'/>\n";
      xml << "    <sr mask='001000' test='001000'/>\n";
      xml << "    <map address='00-1f:6000-7fff'/>\n";
      xml << "    <map address='80-9f:6000-7fff'/>\n";
    }
    xml << "  </necdsp>\n";
  }

  if(has_dsp2) {
    xml << "  <necdsp revision='upd7725' frequency='8000000' program='dsp2.bin' sha256='5efbdf96ed0652790855225964f3e90e6a4d466cfa64df25b110933c6cf94ea1'>\n";
    xml << "    <dr mask='004000' test='000000'/>\n";
    xml << "    <sr mask='004000' test='004000'/>\n";
    xml << "    <map address='20-3f:8000-ffff'/>\n";
    xml << "    <map address='a0-bf:8000-ffff'/>\n";
    xml << "  </necdsp>\n";
  }

  if(has_dsp3) {
    xml << "  <necdsp revision='upd7725' frequency='8000000' program='dsp3.bin' sha256='2e635f72e4d4681148bc35429421c9b946e4f407590e74e31b93b8987b63ba90'>\n";
    xml << "    <dr mask='004000' test='000000'/>\n";
    xml << "    <sr mask='004000' test='004000'/>\n";
    xml << "    <map address='20-3f:8000-ffff'/>\n";
    xml << "    <map address='a0-bf:8000-ffff'/>\n";
    xml << "  </necdsp>\n";
  }

  if(has_dsp4) {
    xml << "  <necdsp revision='upd7725' frequency='8000000' program='dsp4.bin' sha256='63ede17322541c191ed1fdf683872554a0a57306496afc43c59de7c01a6e764a'>\n";
    xml << "    <dr mask='004000' test='000000'/>\n";
    xml << "    <sr mask='004000' test='004000'/>\n";
    xml << "    <map address='30-3f:8000-ffff'/>\n";
    xml << "    <map address='b0-bf:8000-ffff'/>\n";
    xml << "  </necdsp>\n";
  }

  if(has_obc1) {
    xml << "  <obc1>\n";
    xml << "    <map address='00-3f:6000-7fff'/>\n";
    xml << "    <map address='80-bf:6000-7fff'/>\n";
    xml << "  </obc1>\n";
  }

  if(has_st010) {
    xml << "  <necdsp revision='upd96050' frequency='10000000' program='st0010.bin' sha256='55c697e864562445621cdf8a7bf6e84ae91361e393d382a3704e9aa55559041e'>\n";
    xml << "    <dr mask='080001' test='000000'/>\n";
    xml << "    <sr mask='080001' test='000001'/>\n";
    xml << "    <dp mask='080000' test='080000'/>\n";
    xml << "    <map address='60-6f:0000-0fff'/>\n";
    xml << "    <map address='e0-ef:0000-0fff'/>\n";
    xml << "  </necdsp>\n";
  }

  if(has_st011) {
    xml << "  <necdsp revision='upd96050' frequency='15000000' program='st0011.bin' sha256='651b82a1e26c4fa8dd549e91e7f923012ed2ca54c1d9fd858655ab30679c2f0e'>\n";
    xml << "    <dr mask='080001' test='000000'/>\n";
    xml << "    <sr mask='080001' test='000001'/>\n";
    xml << "    <dp mask='080000' test='080000'/>\n";
    xml << "    <map address='60-6f:0000-0fff'/>\n";
    xml << "    <map address='e0-ef:0000-0fff'/>\n";
    xml << "  </necdsp>\n";
  }

  if(has_st018) {
    xml << "  <setarisc program='ST-0018'>\n";
    xml << "    <map address='00-3f:3800-38ff'/>\n";
    xml << "    <map address='80-bf:3800-38ff'/>\n";
    xml << "  </setarisc>\n";
  }

  // always set dos default mapping if none applied
  has_dos = true;
  if (has_dos) {
    xml << "  <dos>\n";
    xml << "    <map address='00-3f:5f00-5fff'/>\n";
    xml << "    <map address='80-bf:5f00-5fff'/>\n";
    xml << "  </dos>\n";
  }

  xml << "</cartridge>\n";
  xmlMemoryMap = xml;
}

void SNESCartridge::read_header(const uint8_t *data, unsigned size) {
  type         = TypeUnknown;
  mapper       = LoROM;
  dsp1_mapper  = DSP1Unmapped;
  bsxpack_type = FlashROM;
  region       = NTSC;
  rom_size     = size;
  ram_size     = 0;

  has_bsx_slot   = false;
  has_spc7110rtc = false;
  has_srtc       = false;
  has_dsp1       = false;
  has_dsp2       = false;
  has_dsp3       = false;
  has_dsp4       = false;
  has_obc1       = false;
  has_st010      = false;
  has_st011      = false;
  has_st018      = false;
  has_dos        = false;

  //=====================
  //detect Game Boy carts
  //=====================

  if(size >= 0x0140) {
    if(data[0x0104] == 0xce && data[0x0105] == 0xed && data[0x0106] == 0x66 && data[0x0107] == 0x66
    && data[0x0108] == 0xcc && data[0x0109] == 0x0d && data[0x010a] == 0x00 && data[0x010b] == 0x0b) {
      type = TypeGameBoy;
      return;
    }
  }

  if(size < 32768) {
    type = TypeUnknown;
    return;
  }

  const unsigned index = find_header(data, size);
  const uint8_t mapperid = data[index + Mapper];
  const uint8_t rom_type = data[index + RomType];
  const uint8_t rom_size = data[index + RomSize];
  const uint8_t company  = data[index + Company];
  const uint8_t regionid = data[index + CartRegion] & 0x7f;

  ram_size = 1024 << (data[index + RamSize] & 7);
  if(ram_size == 1024) ram_size = 0;  //no RAM present

  //0, 1, 11, 13, 15, 16 = NTSC; others = PAL
  switch (regionid) {
  case 0: case 1: case 11: case 13: case 15: case 16:
    region = NTSC;
    break;
  default:
    region = PAL;
    break;
  }

  //=======================
  //detect BS-X flash carts
  //=======================

  if(data[index + 0x13] == 0x00 || data[index + 0x13] == 0xff) {
    if(data[index + 0x14] == 0x00) {
      const uint8_t n15 = data[index + 0x15];
      if(n15 == 0x00 || n15 == 0x80 || n15 == 0x84 || n15 == 0x9c || n15 == 0xbc || n15 == 0xfc) {
        if(data[index + 0x1a] == 0x33 || data[index + 0x1a] == 0xff) {
          type = TypeBsx;
          //Check if FlashROM or MaskROM
          uint8_t i = 0;
          for (i = 0; i < 20; i++)
          {
            uint8_t checkbyte;
            switch(i)
            {
              case 0x00: checkbyte = 0x4D; break;
              case 0x02: checkbyte = 0x50; break;
              case 0x06: checkbyte = 0x70; break;
              default:   checkbyte = 0x00;
            }

            if (i != 0x06)
            {
              if (data[index - 0xC0 + i] != checkbyte)
              {
                break;
              }
            }
            else
            {
              //Only check 0xF0 for i = 6, only Memory Pack type matters
              if ((data[index - 0xC0 + i] & 0xF0) != checkbyte)
              {
                break;
              }
            }
          }

          if (i == 20)
          {
            //if i reaches 20, that means all the checks are successful
            bsxpack_type = MaskROM;
          }
          else
          {
            bsxpack_type = FlashROM;
          }
          region = NTSC;  //BS-X only released in Japan
          return;
        }
      }
    }
  }

  //=========================
  //detect Sufami Turbo carts
  //=========================

  if(!memcmp(data, "BANDAI SFC-ADX", 14)) {
    if(!memcmp(data + 16, "SFC-ADX BACKUP", 14)) {
      type = TypeSufamiTurboBios;
    } else {
      type = TypeSufamiTurbo;
    }
    mapper = STROM;
    region = NTSC;  //Sufami Turbo only released in Japan
    return;         //RAM size handled outside this routine
  }

  //==========================
  //detect Super Game Boy BIOS
  //==========================

  if(!memcmp(data + index, "Super GAMEBOY2", 14)) {
    type = TypeSuperGameBoy2Bios;
    mapper = SGBROM;
    return;
  }

  if(!memcmp(data + index, "Super GAMEBOY", 13)) {
    type = TypeSuperGameBoy1Bios;
    mapper = SGBROM;
    return;
  }

  //=====================
  //detect standard carts
  //=====================

  //detect presence of BS-X flash cartridge connector (reads extended header information)
  if(data[index - 14] == 'Z') {
    if(data[index - 11] == 'J') {
      uint8_t n13 = data[index - 13];
      if((n13 >= 'A' && n13 <= 'Z') || (n13 >= '0' && n13 <= '9')) {
        if(company == 0x33 || (data[index - 10] == 0x00 && data[index - 4] == 0x00)) {
          has_bsx_slot = true;
        }
      }
    }
  }

  if(has_bsx_slot) {
    if(!memcmp(data + index, "Satellaview BS-X     ", 21)) {
      //BS-X base cart
      type = TypeBsxBios;
      mapper = BSXROM;
      region = NTSC;  //BS-X only released in Japan
      return;         //RAM size handled internally by load_cart_bsx() -> BSXCart class
    } else {
      type = TypeBsxSlotted;
      mapper = (index == 0x7fc0 ? BSCLoROM : BSCHiROM);
      region = NTSC;  //BS-X slotted cartridges only released in Japan
    }
  } else {
    //standard cart
    type = TypeNormal;

    if(index == 0x7fc0) {
      mapper = LoROM;
    } else if(index == 0xffc0) {
      mapper = HiROM;
    } else {  //index == 0x40ffc0
      mapper = ExHiROM;
    }
  }

  if(mapperid == 0x20 && (rom_type == 0x13 || rom_type == 0x14 || rom_type == 0x15 || rom_type == 0x1a)) {
    mapper = SuperFXROM;
    ram_size = 1024 << (data[index - 3] & 7);
    if(ram_size == 1024) ram_size = 0;
  }

  if(mapperid == 0x23 && (rom_type == 0x32 || rom_type == 0x34 || rom_type == 0x35 || rom_type == 0x36)) {
    mapper = SA1ROM;
  }

  if(mapperid == 0x32 && (rom_type == 0x43 || rom_type == 0x45)) {
    mapper = SDD1ROM;
  }

  if(mapperid == 0x3a && (rom_type == 0xf5 || rom_type == 0xf9)) {
    mapper = SPC7110ROM;
    has_spc7110rtc = (rom_type == 0xf9);
  }

  if(mapperid == 0x35 && rom_type == 0x55) {
    has_srtc = true;
  }

  if(mapperid == 0x20 && rom_type == 0xf3) {
    mapper = Cx4ROM;
  }

  if((mapperid == 0x20 || mapperid == 0x21) && rom_type == 0x03) {
    has_dsp1 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x05 && company != 0xb2) {
    has_dsp1 = true;
  }

  if(mapperid == 0x31 && (rom_type == 0x03 || rom_type == 0x05)) {
    has_dsp1 = true;
  }

  if(has_dsp1) {
    if((mapperid & 0x2f) == 0x20 && size <= 0x100000) {
      dsp1_mapper = DSP1LoROM1MB;
    } else if((mapperid & 0x2f) == 0x20) {
      dsp1_mapper = DSP1LoROM2MB;
    } else if((mapperid & 0x2f) == 0x21) {
      dsp1_mapper = DSP1HiROM;
    }
  }

  if(mapperid == 0x20 && rom_type == 0x05) {
    has_dsp2 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x05 && company == 0xb2) {
    has_dsp3 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x03) {
    has_dsp4 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x25) {
    has_obc1 = true;
  }

  if(mapperid == 0x30 && rom_type == 0xf6 && rom_size >= 10) {
    has_st010 = true;
  }

  if(mapperid == 0x30 && rom_type == 0xf6 && rom_size < 10) {
    has_st011 = true;
  }

  if(mapperid == 0x30 && rom_type == 0xf5) {
    has_st018 = true;
  }
}

unsigned SNESCartridge::find_header(const uint8_t *data, unsigned size) {
  unsigned score_lo = score_header(data, size, 0x007fc0);
  unsigned score_hi = score_header(data, size, 0x00ffc0);
  unsigned score_ex = score_header(data, size, 0x40ffc0);
  if(score_ex) score_ex += 4;  //favor ExHiROM on images > 32mbits

  if(score_lo >= score_hi && score_lo >= score_ex) {
    return 0x007fc0;
  } else if(score_hi >= score_ex) {
    return 0x00ffc0;
  } else {
    return 0x40ffc0;
  }
}

unsigned SNESCartridge::score_header(const uint8_t *data, unsigned size, unsigned addr) {
  if(size < addr + 64) return 0;  //image too small to contain header at this location?
  int score = 0;

  uint16_t resetvector = data[addr + ResetVector] | (data[addr + ResetVector + 1] << 8);
  uint16_t checksum    = data[addr + Checksum   ] | (data[addr + Checksum    + 1] << 8);
  uint16_t complement  = data[addr + Complement ] | (data[addr + Complement  + 1] << 8);

  uint8_t resetop = data[(addr & ~0x7fff) | (resetvector & 0x7fff)];  //first opcode executed upon reset
  uint8_t mapper  = data[addr + Mapper] & ~0x10;                      //mask off irrelevent FastROM-capable bit

  //$00:[0000-7fff] contains uninitialized RAM and MMIO.
  //reset vector must point to ROM at $00:[8000-ffff] to be considered valid.
  if(resetvector < 0x8000) return 0;

  //some images duplicate the header in multiple locations, and others have completely
  //invalid header information that cannot be relied upon.
  //below code will analyze the first opcode executed at the specified reset vector to
  //determine the probability that this is the correct header.

  //most likely opcodes
  if(resetop == 0x78  //sei
  || resetop == 0x18  //clc (clc; xce)
  || resetop == 0x38  //sec (sec; xce)
  || resetop == 0x9c  //stz $nnnn (stz $4200)
  || resetop == 0x4c  //jmp $nnnn
  || resetop == 0x5c  //jml $nnnnnn
  ) score += 8;

  //plausible opcodes
  if(resetop == 0xc2  //rep #$nn
  || resetop == 0xe2  //sep #$nn
  || resetop == 0xad  //lda $nnnn
  || resetop == 0xae  //ldx $nnnn
  || resetop == 0xac  //ldy $nnnn
  || resetop == 0xaf  //lda $nnnnnn
  || resetop == 0xa9  //lda #$nn
  || resetop == 0xa2  //ldx #$nn
  || resetop == 0xa0  //ldy #$nn
  || resetop == 0x20  //jsr $nnnn
  || resetop == 0x22  //jsl $nnnnnn
  ) score += 4;

  //implausible opcodes
  if(resetop == 0x40  //rti
  || resetop == 0x60  //rts
  || resetop == 0x6b  //rtl
  || resetop == 0xcd  //cmp $nnnn
  || resetop == 0xec  //cpx $nnnn
  || resetop == 0xcc  //cpy $nnnn
  ) score -= 4;

  //least likely opcodes
  if(resetop == 0x00  //brk #$nn
  || resetop == 0x02  //cop #$nn
  || resetop == 0xdb  //stp
  || resetop == 0x42  //wdm
  || resetop == 0xff  //sbc $nnnnnn,x
  ) score -= 8;

  //at times, both the header and reset vector's first opcode will match ...
  //fallback and rely on info validity in these cases to determine more likely header.

  //a valid checksum is the biggest indicator of a valid header.
  if((checksum + complement) == 0xffff && (checksum != 0) && (complement != 0)) score += 4;

  if(addr == 0x007fc0 && mapper == 0x20) score += 2;  //0x20 is usually LoROM
  if(addr == 0x00ffc0 && mapper == 0x21) score += 2;  //0x21 is usually HiROM
  if(addr == 0x007fc0 && mapper == 0x22) score += 2;  //0x22 is usually SDD1
  if(addr == 0x40ffc0 && mapper == 0x25) score += 2;  //0x25 is usually ExHiROM

  if(data[addr + Company] == 0x33) score += 2;        //0x33 indicates extended header
  if(data[addr + RomType] < 0x08) score++;
  if(data[addr + RomSize] < 0x10) score++;
  if(data[addr + RamSize] < 0x08) score++;
  if(data[addr + CartRegion] < 14) score++;

  if(score < 0) score = 0;
  return score;
}

unsigned SNESCartridge::gameboy_ram_size(const uint8_t *data, unsigned size) {
  if(size < 512) return 0;
  if(data[0x0147] == 0x06) return 512;  //MBC2 has 512 nibbles of internal RAM
  switch(data[0x0149]) {
    case 0x00: return   0 * 1024;
    case 0x01: return   2 * 1024;
    case 0x02: return   8 * 1024;
    case 0x03: return  32 * 1024;
    case 0x04: return 128 * 1024;
    case 0x05: return 128 * 1024;
    default:   return 128 * 1024;
  }
}

bool SNESCartridge::gameboy_has_rtc(const uint8_t *data, unsigned size) {
  if(size < 512) return false;
  if(data[0x0147] == 0x0f || data[0x0147] == 0x10) return true;
  return false;
}

unsigned SNESCartridge::sufamiturbo_ram_size(const uint8_t *data, unsigned size) {
  if(size < 0x38) return 0;
  return data[0x37] * 2048;
}

}

#endif
