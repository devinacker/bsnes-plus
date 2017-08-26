#ifdef CARTRIDGE_CPP

void Cartridge::parse_xml(const lstring &list) {
  mapping.reset();

  //parse any slots *before* parsing the base cartridge
  if(mode == Mode::BsxSlotted) {
    parse_xml_bsx(list[1]);
  } else if(mode == Mode::Bsx) {
    parse_xml_bsx(list[1]);
  } else if(mode == Mode::SufamiTurbo) {
    parse_xml_sufami_turbo(list[1], 0);
    parse_xml_sufami_turbo(list[2], 1);
  } else if(mode == Mode::SuperGameBoy) {
    parse_xml_gameboy(list[1]);
  }

  parse_xml_cartridge(list[0]);
}

void Cartridge::parse_xml_cartridge(const char *data) {
  xml_element document = xml_parse(data);
  if(document.element.size() == 0) return;

  foreach(head, document.element) {
    if(head.name == "cartridge") {
      foreach(attr, head.attribute) {
        if(attr.name == "region") {
          if(attr.content == "NTSC") region = Region::NTSC;
          if(attr.content == "PAL") region = Region::PAL;
        }
      }

      foreach(node, head.element) {
        if(node.name == "rom") xml_parse_rom(node);
        if(node.name == "ram") xml_parse_ram(node);
        if(node.name == "superfx") xml_parse_superfx(node);
        if(node.name == "sa1") xml_parse_sa1(node);
        if(node.name == "necdsp") xml_parse_necdsp(node);
        if(node.name == "bsx") xml_parse_bsx(node);
        if(node.name == "sufamiturbo") xml_parse_sufamiturbo(node);
        if(node.name == "supergameboy") xml_parse_supergameboy(node);
        if(node.name == "srtc") xml_parse_srtc(node);
        if(node.name == "sdd1") xml_parse_sdd1(node);
        if(node.name == "spc7110") xml_parse_spc7110(node);
        if(node.name == "cx4") xml_parse_cx4(node);
        if(node.name == "obc1") xml_parse_obc1(node);
        if(node.name == "setarisc") xml_parse_setarisc(node);
        if(node.name == "msu1") xml_parse_msu1(node);
        if(node.name == "serial") xml_parse_serial(node);
      }
    }
  }
}

void Cartridge::parse_xml_bsx(const char *data) {
  xml_element document = xml_parse(data);
  if(document.element.size() == 0) return;

  foreach(head, document.element) {
    if(head.name == "cartridge") {
      foreach(attr, head.attribute) {
        if(attr.name == "type") {
          if(attr.content == "FlashROM") bsxpack_type = BSXPackType::FlashROM;
          if(attr.content == "MaskROM") bsxpack_type = BSXPackType::MaskROM;
        }
      }
    }
  }
}

void Cartridge::parse_xml_sufami_turbo(const char *data, bool slot) {
  xml_element document = xml_parse(data);
  if(document.element.size() == 0) return;

  foreach(head, document.element) {
    if(head.name == "cartridge") {
      foreach(leaf, head.element) {
        if(leaf.name == "ram") {
          foreach(attr, leaf.attribute) {
            if(attr.name == "size") {
              (slot == 0 ? st_A_ram_size : st_B_ram_size) = hex(attr.content);
            }
          }
        }
      }
    }
  }
}

void Cartridge::parse_xml_gameboy(const char *data) {
  xml_element document = xml_parse(data);
  if(document.element.size() == 0) return;

  foreach(head, document.element) {
    if(head.name == "cartridge") {
      foreach(attr, head.attribute) {
        if(attr.name == "rtc") {
          supergameboy_rtc_size = (attr.content == "true") ? 4 : 0;
        }
      }

      foreach(leaf, head.element) {
        if(leaf.name == "ram") {
          foreach(attr, leaf.attribute) {
            if(attr.name == "size") {
              supergameboy_ram_size = hex(attr.content);
            }
          }
        }
      }
    }
  }
}

void Cartridge::xml_parse_memory(xml_element &root, Memory &memory) {
  foreach(leaf, root.element) {
    if(leaf.name == "map") {
      Mapping m(memory);
      foreach(attr, leaf.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
        if(attr.name == "mode") xml_parse_mode(m, attr.content);
        if(attr.name == "offset") m.offset = hex(attr.content);
        if(attr.name == "size") m.size = hex(attr.content);
      }
      mapping.append(m);
    }
  }
}

void Cartridge::xml_parse_rom(xml_element &root) {
  xml_parse_memory(root, memory::cartrom);
}

void Cartridge::xml_parse_ram(xml_element &root) {
  foreach(attr, root.attribute) {
    if(attr.name == "size") ram_size = hex(attr.content);
  }
  if(ram_size > 0) {
    xml_parse_memory(root, memory::cartram);
  }
}

void Cartridge::xml_parse_superfx(xml_element &root) {
  has_superfx = true;

  foreach(node, root.element) {
    if(node.name == "rom") {
      xml_parse_memory(node, memory::fxrom);
    } else if(node.name == "ram") {
      foreach(attr, node.attribute) {
        if(attr.name == "size") ram_size = hex(attr.content);
      }
      if(ram_size > 0) {
        xml_parse_memory(node, memory::fxram);
      }
    } else if(node.name == "mmio") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(superfx);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::xml_parse_sa1(xml_element &root) {
  has_sa1 = true;

  foreach(node, root.element) {
    if(node.name == "rom") {
      xml_parse_memory(node, memory::vsprom);
    } else if(node.name == "iram") {
      xml_parse_memory(node, memory::cpuiram);
    } else if(node.name == "bwram") {
      foreach(attr, node.attribute) {
        if(attr.name == "size") ram_size = hex(attr.content);
      }
      if (ram_size) {
        xml_parse_memory(node, memory::cc1bwram);
      }
    } else if(node.name == "mmio") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(sa1);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::xml_parse_necdsp(xml_element &root) {
  has_necdsp = true;
  necdsp.revision = NECDSP::Revision::uPD7725;
  necdsp.frequency = 8000000;

  for(unsigned n = 0; n < 16384; n++) necdsp.programROM[n] = 0x000000;
  for(unsigned n = 0; n <  2048; n++) necdsp.dataROM[n] = 0x0000;

  string program, programhash;
  string sha256;

  foreach(attr, root.attribute) {
    if(attr.name == "revision") {
      if(attr.content == "upd7725" ) necdsp.revision = NECDSP::Revision::uPD7725;
      if(attr.content == "upd96050") necdsp.revision = NECDSP::Revision::uPD96050;
    } else if(attr.name == "frequency") {
      necdsp.frequency = decimal(attr.content);
    } else if(attr.name == "program") {
      program = attr.content;
    } else if(attr.name == "sha256") {
      sha256 = attr.content;
    }
  }

  unsigned promsize = (necdsp.revision == NECDSP::Revision::uPD7725 ? 2048 : 16384);
  unsigned dromsize = (necdsp.revision == NECDSP::Revision::uPD7725 ? 1024 :  2048);
  unsigned filesize = promsize * 3 + dromsize * 2;

  file fp;
  if(fp.open(string(dir(basename()), program), file::mode::read)) {
    if(fp.size() == filesize) {
      for(unsigned n = 0; n < promsize; n++) necdsp.programROM[n] = fp.readm(3);
      for(unsigned n = 0; n < dromsize; n++) necdsp.dataROM[n] = fp.readm(2);

      fp.seek(0);
      uint8_t data[filesize];
      fp.read(data, filesize);

      sha256_ctx sha;
      uint8 shahash[32];
      sha256_init(&sha);
      sha256_chunk(&sha, data, filesize);
      sha256_final(&sha);
      sha256_hash(&sha, shahash);
      foreach(n, shahash) programhash.append(hex<2>(n));
    }
    fp.close();
  }

  foreach(node, root.element) {
    if(node.name == "dr") {
      foreach(attr, node.attribute) {
        if(attr.name == "mask") necdsp.drmask = hex(attr.content);
        if(attr.name == "test") necdsp.drtest = hex(attr.content);
      }
    }

    if(node.name == "sr") {
      foreach(attr, node.attribute) {
        if(attr.name == "mask") necdsp.srmask = hex(attr.content);
        if(attr.name == "test") necdsp.srtest = hex(attr.content);
      }
    }

    if(node.name == "dp") {
      foreach(attr, node.attribute) {
        if(attr.name == "mask") necdsp.dpmask = hex(attr.content);
        if(attr.name == "test") necdsp.dptest = hex(attr.content);
      }
    }

    if(node.name == "map") {
      Mapping m(necdsp);
      foreach(attr, node.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
      }
      mapping.append(m);
    }
  }

  if(programhash == "") {
    system.interface->message({ "Warning: NEC DSP program ", program, " is missing." });
  } else if(sha256 != "" && sha256 != programhash) {
    system.interface->message({
      "Warning: NEC DSP program ", program, " SHA256 is incorrect.\n\n"
      "Expected:\n", sha256, "\n\n"
      "Actual:\n", programhash
    });
  }
}

void Cartridge::xml_parse_bsx(xml_element &root) {
  has_bsx_slot = true;
  if(mode != Mode::BsxSlotted && mode != Mode::Bsx) return;

  foreach(node, root.element) {
    if(node.name == "slot") {
      xml_parse_memory(node, bsxpack_access());
    } else if(node.name == "mcc") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(bsxcart);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::xml_parse_sufamiturbo(xml_element &root) {
  if(mode != Mode::SufamiTurbo) return;

  foreach(node, root.element) {
    if(node.name == "slot") {
      bool slotid = 0;
      foreach(attr, node.attribute) {
        if(attr.name == "id") {
          if(attr.content == "A") slotid = 0;
          if(attr.content == "B") slotid = 1;
        }
      }

      Memory &rom = (slotid == 0) ? memory::stArom : memory::stBrom;
      if(rom.size() == 0) continue;
      Memory &ram = (slotid == 0) ? memory::stAram : memory::stBram;
      unsigned ram_size = (slotid == 0) ? st_A_ram_size : st_B_ram_size;

      foreach(slot, node.element) {
        if(slot.name == "rom") {
          xml_parse_memory(slot, rom);
        } else if(slot.name == "ram" && ram_size > 0) {
          xml_parse_memory(slot, ram);
        }
      }
    }
  }
}

void Cartridge::xml_parse_supergameboy(xml_element &root) {
  if(mode != Mode::SuperGameBoy) return;

  foreach(attr, root.attribute) {
    if(attr.name == "revision") {
      if(attr.content == "1") supergameboy_version = SuperGameBoyVersion::Version1;
      if(attr.content == "2") supergameboy_version = SuperGameBoyVersion::Version2;
    }
  }

  foreach(node, root.element) {
    if(node.name == "map") {
      Mapping m((Memory&)supergameboy);
      foreach(attr, node.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
      }
      mapping.append(m);
    }
  }
}

void Cartridge::xml_parse_srtc(xml_element &root) {
  has_srtc = true;

  foreach(node, root.element) {
    if(node.name == "map") {
      Mapping m(srtc);
      foreach(attr, node.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
      }
      mapping.append(m);
    }
  }
}

void Cartridge::xml_parse_sdd1(xml_element &root) {
  has_sdd1 = true;

  foreach(node, root.element) {
    if(node.name == "mcu") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m((Memory&)sdd1);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    } else if(node.name == "mmio") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m((MMIO&)sdd1);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::xml_parse_spc7110(xml_element &root) {
  has_spc7110 = true;

  foreach(node, root.element) {
    if(node.name == "dcu") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(spc7110dcu);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    } else if(node.name == "mcu") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(spc7110mcu);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
            if(attr.name == "offset") spc7110_data_rom_offset = hex(attr.content);
          }
          mapping.append(m);
        }
      }
    } else if(node.name == "mmio") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(spc7110);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    } else if(node.name == "ram") {
      foreach(attr, node.attribute) {
        if(attr.name == "size") ram_size = hex(attr.content);
      }
      if(ram_size > 0) {
        xml_parse_memory(node, spc7110ram);
      }
    } else if(node.name == "rtc") {
      has_spc7110rtc = true;

      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(spc7110);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::xml_parse_cx4(xml_element &root) {
  has_cx4 = true;
  cx4.frequency = 20000000;

  // TODO: allow custom data ROM, maybe

  foreach(attr, root.attribute) {
    if(attr.name == "frequency") {
      cx4.frequency = decimal(attr.content);
    }
  }
  
  foreach(node, root.element) {
    if(node.name == "rom") {
      xml_parse_memory(node, memory::cx4rom);
    } else if(node.name == "ram") {
      xml_parse_memory(node, memory::cx4ram);
    } else if(node.name == "map") {
      Mapping m(cx4);
      foreach(attr, node.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
      }
      mapping.append(m);
    }
  }
}

void Cartridge::xml_parse_obc1(xml_element &root) {
  has_obc1 = true;

  foreach(node, root.element) {
    if(node.name == "map") {
      Mapping m(obc1);
      foreach(attr, node.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
      }
      m.min_ram_size = 0x2000;
      mapping.append(m);
    }
  }
}

void Cartridge::xml_parse_setarisc(xml_element &root) {
  unsigned program = 0;

  foreach(attr, root.attribute) {
    if(attr.name == "program") {
      if(attr.content == "ST-0018") {
        program = 1;
        has_st0018 = true;
      }
    }
  }

  MMIO *map[2] = { 0, &st0018 };

  foreach(node, root.element) {
    if(node.name == "map" && map[program]) {
      Mapping m(*map[program]);
      foreach(attr, node.attribute) {
        if(attr.name == "address") xml_parse_address(m, attr.content);
      }
      mapping.append(m);
    }
  }
}

void Cartridge::xml_parse_msu1(xml_element &root) {
  has_msu1 = true;

  foreach(node, root.element) {
    if(node.name == "mmio") {
      foreach(leaf, node.element) {
        if(leaf.name == "map") {
          Mapping m(msu1);
          foreach(attr, leaf.attribute) {
            if(attr.name == "address") xml_parse_address(m, attr.content);
          }
          mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::xml_parse_serial(xml_element &root) {
  has_serial = true;
}

void Cartridge::xml_parse_address(Mapping &m, const string &data) {
  lstring part;
  part.split(":", data);
  if(part.size() != 2) return;

  lstring subpart;
  subpart.split("-", part[0]);
  if(subpart.size() == 1) {
    m.banklo = hex(subpart[0]);
    m.bankhi = m.banklo;
  } else if(subpart.size() == 2) {
    m.banklo = hex(subpart[0]);
    m.bankhi = hex(subpart[1]);
  }

  subpart.split("-", part[1]);
  if(subpart.size() == 1) {
    m.addrlo = hex(subpart[0]);
    m.addrhi = m.addrlo;
  } else if(subpart.size() == 2) {
    m.addrlo = hex(subpart[0]);
    m.addrhi = hex(subpart[1]);
  }
}

void Cartridge::xml_parse_mode(Mapping &m, const string& data) {
       if(data == "direct") m.mode = Bus::MapMode::Direct;
  else if(data == "linear") m.mode = Bus::MapMode::Linear;
  else if(data == "shadow") m.mode = Bus::MapMode::Shadow;
}

Cartridge::Mapping::Mapping() {
  memory = 0;
  mmio = 0;
  mode = Bus::MapMode::Direct;
  banklo = bankhi = addrlo = addrhi = offset = size = min_ram_size = 0;
}

Cartridge::Mapping::Mapping(Memory &memory_) {
  memory = &memory_;
  mmio = 0;
  mode = Bus::MapMode::Direct;
  banklo = bankhi = addrlo = addrhi = offset = size = min_ram_size = 0;
}

Cartridge::Mapping::Mapping(MMIO &mmio_) {
  memory = 0;
  mmio = &mmio_;
  mode = Bus::MapMode::Direct;
  banklo = bankhi = addrlo = addrhi = offset = size = min_ram_size = 0;
}

#endif
