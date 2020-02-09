SuperGameBoy supergameboy;

#include <nall/snes/sgb.hpp>

static void op_step_default(uint16_t pc) {
  uint8_t op  = supergameboy.read_gb(pc);
  uint8_t op1 = supergameboy.read_gb(pc + 1);
  uint8_t op2 = supergameboy.read_gb(pc + 2);
  
  printf("%04x %s\n", pc, GBCPU::disassemble(pc, op, op1, op2)());
}

static void op_read_default(uint16_t addr, uint8_t data) {
  printf("op_read  %04x => %02x\n", addr, data);
}
static void op_write_default(uint16_t addr, uint8_t data) {
  printf("op_write %04x <= %02x\n", addr, data);
}

const uint8_t SuperGameBoy::bootroms[2][256] =
{
/* SGB1 boot ROM */
{
  0x31, 0xfe, 0xff, 0x3e, 0x30, 0xe0, 0x00, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb,
  0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77,
  0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x21, 0x5f, 0xc0, 0x0e, 0x08, 0xaf, 0x32, 0x0d, 0x20, 0xfc, 0x11,
  0x4f, 0x01, 0x3e, 0xfb, 0x0e, 0x06, 0xf5, 0x06, 0x00, 0x1a, 0x1b, 0x32, 0x80, 0x47, 0x0d, 0x20,
  0xf8, 0x32, 0xf1, 0x32, 0x0e, 0x0e, 0xd6, 0x02, 0xfe, 0xef, 0x20, 0xea, 0x11, 0x04, 0x01, 0x21,
  0x10, 0x80, 0x1a, 0xcd, 0xd3, 0x00, 0xcd, 0xd4, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11,
  0xe6, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99,
  0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3,
  0x3e, 0x91, 0xe0, 0x40, 0x21, 0x00, 0xc0, 0x0e, 0x00, 0x3e, 0x00, 0xe2, 0x3e, 0x30, 0xe2, 0x06,
  0x10, 0x1e, 0x08, 0x2a, 0x57, 0xcb, 0x42, 0x3e, 0x10, 0x20, 0x02, 0x3e, 0x20, 0xe2, 0x3e, 0x30,
  0xe2, 0xcb, 0x1a, 0x1d, 0x20, 0xef, 0x05, 0x20, 0xe8, 0x3e, 0x20, 0xe2, 0x3e, 0x30, 0xe2, 0xcd,
  0xc2, 0x00, 0x7d, 0xfe, 0x60, 0x20, 0xd2, 0x0e, 0x13, 0x3e, 0xc1, 0xe2, 0x0c, 0x3e, 0x07, 0xe2,
  0x18, 0x3a, 0x16, 0x04, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x1e, 0x00, 0x1d, 0x20, 0xfd, 0x15,
  0x20, 0xf2, 0xc9, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20,
  0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x01, 0xe0, 0x50
},
/* SGB2 boot ROM */
{
  0x31, 0xfe, 0xff, 0x3e, 0x30, 0xe0, 0x00, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb,
  0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77,
  0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x21, 0x5f, 0xc0, 0x0e, 0x08, 0xaf, 0x32, 0x0d, 0x20, 0xfc, 0x11,
  0x4f, 0x01, 0x3e, 0xfb, 0x0e, 0x06, 0xf5, 0x06, 0x00, 0x1a, 0x1b, 0x32, 0x80, 0x47, 0x0d, 0x20,
  0xf8, 0x32, 0xf1, 0x32, 0x0e, 0x0e, 0xd6, 0x02, 0xfe, 0xef, 0x20, 0xea, 0x11, 0x04, 0x01, 0x21,
  0x10, 0x80, 0x1a, 0xcd, 0xd3, 0x00, 0xcd, 0xd4, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11,
  0xe6, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99,
  0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3,
  0x3e, 0x91, 0xe0, 0x40, 0x21, 0x00, 0xc0, 0x0e, 0x00, 0x3e, 0x00, 0xe2, 0x3e, 0x30, 0xe2, 0x06,
  0x10, 0x1e, 0x08, 0x2a, 0x57, 0xcb, 0x42, 0x3e, 0x10, 0x20, 0x02, 0x3e, 0x20, 0xe2, 0x3e, 0x30,
  0xe2, 0xcb, 0x1a, 0x1d, 0x20, 0xef, 0x05, 0x20, 0xe8, 0x3e, 0x20, 0xe2, 0x3e, 0x30, 0xe2, 0xcd,
  0xc2, 0x00, 0x7d, 0xfe, 0x60, 0x20, 0xd2, 0x0e, 0x13, 0x3e, 0xc1, 0xe2, 0x0c, 0x3e, 0x07, 0xe2,
  0x18, 0x3a, 0x16, 0x04, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x1e, 0x00, 0x1d, 0x20, 0xfd, 0x15,
  0x20, 0xf2, 0xc9, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20,
  0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0xff, 0xe0, 0x50
}
};

//====================
//SuperGameBoy::Packet
//====================

const char SuperGameBoy::command_name[32][64] = {
  "PAL01",    "PAL23",    "PAL03",    "PAL12",
  "ATTR_BLK", "ATTR_LIN", "ATTR_DIV", "ATTR_CHR",
  "SOUND",    "SOU_TRN",  "PAL_SET",  "PAL_TRN",
  "ATRC_EN",  "TEST_EN",  "ICON_EN",  "DATA_SND",
  "DATA_TRN", "MLT_REQ",  "JUMP",     "CHR_TRN",
  "PCT_TRN",  "ATTR_TRN", "ATTR_SET", "MASK_EN",
  "OBJ_TRN",  "19_???",   "1A_???",   "1B_???",
  "1C_???",   "1D_???",   "1E_ROM",   "1F_???",
};

void SuperGameBoy::joyp_write(bool p15, bool p14) {
  //===============
  //joypad handling
  //===============

  if(p15 == 1 && p14 == 1) {
    if(joyp_lock == 0) {
      joyp_lock = 1;
      joyp_id = (joyp_id + 1) & mmio.mlt_req;
    }
  }

  if(p14 == 1 && p15 == 0) joyp_lock ^= 1;

  //===============
  //packet handling
  //===============

  if(p15 == 0 && p14 == 0) {
    //pulse
    pulselock = false;
    packetoffset = 0;
    bitoffset = 0;
    strobelock = true;
    packetlock = false;
    return;
  }

  if(pulselock) return;

  if(p15 == 1 && p14 == 1) {
    strobelock = false;
    return;
  }

  if(strobelock) {
    if(p15 == 1 || p14 == 1) {
      //malformed packet
      packetlock = false;
      pulselock = true;
      bitoffset = 0;
      packetoffset = 0;
    } else {
      return;
    }
  }

  //p15:1, p14:0 = 0
  //p15:0, p14:1 = 1
  bool bit = (p15 == 0);
  strobelock = true;

  if(packetlock) {
    if(p15 == 1 && p14 == 0) {
      if(packetsize < 64) packet[packetsize++] = joyp_packet;
      packetlock = false;
      pulselock = true;
    }
    return;
  }

  bitdata = (bit << 7) | (bitdata >> 1);
  if(++bitoffset < 8) return;

  bitoffset = 0;
  joyp_packet[packetoffset] = bitdata;
  if(++packetoffset < 16) return;
  packetlock = true;
}

//==================
//SuperGameBoy::Core
//==================

static uint8_t null_rom[32768];

bool SuperGameBoy::init(bool version_) {
  if(!romdata) { romdata = null_rom; romsize = 32768; }
  version = version_;

  gambatte_ = new gambatte::GB;
  gambatte_->setInputGetter(this);
  gambatte_->setDebugHandler(this);
  gambatte_->setScanlineCallback(SuperGameBoy::scanline);

  return true;
}

void SuperGameBoy::term() {
  if(gambatte_) {
    delete gambatte_;
    gambatte_ = 0;
  }
}

const uint8_t* SuperGameBoy::bootrom() {
  return bootroms[!!version];
}

unsigned SuperGameBoy::run(uint32_t *samplebuffer, unsigned samples) {
  if((mmio.r6003 & 0x80) == 0) {
    //Gameboy is inactive
    samplebuffer[0] = 0;
    return 1;
  }

  size_t samples_ = samples;
  gambatte_->runFor(buffer, 160, samplebuffer, samples_);
  return samples_;
}

/* static */
void SuperGameBoy::scanline(unsigned line) {
  if (line < 144 && line >> 3 != supergameboy.vram_row) {
    supergameboy.render();
    supergameboy.vram_row = line >> 3;
    supergameboy.mmio.write_buf = (supergameboy.mmio.write_buf + 1) & 3;
  }
}

void SuperGameBoy::save() {
  gambatte_->saveSavedata();
}

void SuperGameBoy::serialize(nall::serializer &s) {
  s.integer(vram_row);
  s.array(vram);

  s.integer(mmio.write_buf);
  s.integer(mmio.read_buf);
  s.integer(mmio.r6003);
  s.integer(mmio.r6004);
  s.integer(mmio.r6005);
  s.integer(mmio.r6006);
  s.integer(mmio.r6007);
  s.array(mmio.r7000);
  s.integer(mmio.read_pos);
  s.integer(mmio.mlt_req);

  for(unsigned i = 0; i < 64; i++) s.array(packet[i].data);
  s.integer(packetsize);

  s.integer(joyp_id);
  s.integer(joyp_lock);
  s.integer(pulselock);
  s.integer(strobelock);
  s.integer(packetlock);
  s.array(joyp_packet.data);
  s.integer(packetoffset);
  s.integer(bitdata);
  s.integer(bitoffset);

  uint8_t *savestate = new uint8_t[256 * 1024];
  if(s.mode() == serializer::Load) {
    s.array(savestate, 256 * 1024);

    file fp;
    if(fp.open("supergameboy-state.tmp", file::mode::write)) {
      fp.write(savestate, 256 * 1024);
      fp.close();

      gambatte_->loadState("supergameboy-state.tmp");
      unlink("supergameboy-state.tmp");
    }
  } else if(s.mode() == serializer::Save) {
    gambatte_->saveState(0, 0, "supergameboy-state.tmp");

    file fp;
    if(fp.open("supergameboy-state.tmp", file::mode::read)) {
      fp.read(savestate, fp.size() < 256 * 1024 ? fp.size() : 256 * 1024);
      fp.close();
    }

    unlink("supergameboy-state.tmp");
    s.array(savestate, 256 * 1024);
  } else if(s.mode() == serializer::Size) {
    s.array(savestate, 256 * 1024);
  }
  delete[] savestate;
}

void SuperGameBoy::power() {
  gambatte_->load(gambatte::GB::FORCE_DMG);
  mmio_reset();
}

void SuperGameBoy::reset() {
  gambatte_->reset();
  mmio_reset();
}

uint8_t SuperGameBoy::read(uint16_t addr) {
  //LY counter
  if(addr == 0x6000) {
    return (vram_row << 3) | mmio.write_buf;
  }

  //command ready port
  if(addr == 0x6002) {
    bool data = packetsize > 0;
    if(data) {
      for(unsigned i = 0; i < 16; i++) mmio.r7000[i] = packet[0][i];
      packetsize--;
      for(unsigned i = 0; i < packetsize; i++) packet[i] = packet[i + 1];
    }
    return data;
  }

  //command port
  if((addr & 0xfff0) == 0x7000) {
    return mmio.r7000[addr & 15];
  }

  if(addr == 0x7800) {
    uint8_t data = vram[mmio.read_buf*320 + mmio.read_pos];
    mmio.read_pos = (mmio.read_pos + 1) % 320;
    return data;
  }

  return 0x00;
}

uint8_t SuperGameBoy::read_gb(uint16_t addr) {
  return gambatte_->debugRead(addr);
}

void SuperGameBoy::write(uint16_t addr, uint8_t data) {
  if(addr == 0x6001) {
    mmio.read_buf = data & 3;
    mmio.read_pos = 0;
  }

  //control port
  //d7 = /RESET line (0 = stop, 1 = run)
  //d5..4 = multiplayer select
  if(addr == 0x6003) {
    if((mmio.r6003 & 0x80) == 0x00 && (data & 0x80) == 0x80) {
      reset();
    }

    mmio.mlt_req = (data & 0x30) >> 4;
    if(mmio.mlt_req == 2) mmio.mlt_req = 3;
    joyp_id &= mmio.mlt_req;

    mmio.r6003 = data;
    return;
  }

  if(addr == 0x6004) { mmio.r6004 = data; return; }  //joypad 1 state
  if(addr == 0x6005) { mmio.r6005 = data; return; }  //joypad 2 state
  if(addr == 0x6006) { mmio.r6006 = data; return; }  //joypad 3 state
  if(addr == 0x6007) { mmio.r6007 = data; return; }  //joypad 4 state
}

void SuperGameBoy::write_gb(uint16_t addr, uint8_t data) {
  gambatte_->debugWrite(addr, data);
}

void SuperGameBoy::mmio_reset() {
  mmio.write_buf = 0;
  mmio.read_buf = 0;
  mmio.r6003 = 0x00;
  mmio.r6004 = 0xff;
  mmio.r6005 = 0xff;
  mmio.r6006 = 0xff;
  mmio.r6007 = 0xff;
  for(unsigned n = 0; n < 16; n++) mmio.r7000[n] = 0;
  mmio.read_pos = 0;
  mmio.mlt_req = 0;

  packetsize = 0;

  vram_row = 0;
  memset(vram, 0, 320*4);

  joyp_id = 0;
  joyp_lock = 0;
  pulselock = true;
}

void SuperGameBoy::render() {
  uint32_t *source = buffer + vram_row * 160 * 8;
  uint8_t *dest = vram + (mmio.write_buf * 320);
  memset(dest, 0x00, 320);

  for(unsigned y = vram_row * 8; y < vram_row * 8 + 8; y++) {
    for(unsigned x = 0; x < 160; x++) {
      unsigned pixel = *source++ / 0x555555;
      pixel ^= 3;

      unsigned addr = (x / 8 * 16) + ((y & 7) * 2);
      dest[addr + 0] |= ((pixel & 1) >> 0) << (7 - (x & 7));
      dest[addr + 1] |= ((pixel & 2) >> 1) << (7 - (x & 7));
    }
  }
}

//==========================
//Gambatte::InputGetter
//==========================

unsigned SuperGameBoy::operator()() {
  unsigned inputState = 0x00;
  unsigned data = 0xFF;
  switch(joyp_id) {
    case 0: data = mmio.r6004; break;
    case 1: data = mmio.r6005; break;
    case 2: data = mmio.r6006; break;
    case 3: data = mmio.r6007; break;
  }
  inputState |= (joyp_id << 8);

  if (!(data & 0x80)) inputState |= gambatte::InputGetter::START;
  if (!(data & 0x40)) inputState |= gambatte::InputGetter::SELECT;
  if (!(data & 0x20)) inputState |= gambatte::InputGetter::B;
  if (!(data & 0x10)) inputState |= gambatte::InputGetter::A;
  if (!(data & 0x08)) inputState |= gambatte::InputGetter::DOWN;
  if (!(data & 0x04)) inputState |= gambatte::InputGetter::UP;
  if (!(data & 0x02)) inputState |= gambatte::InputGetter::LEFT;
  if (!(data & 0x01)) inputState |= gambatte::InputGetter::RIGHT;

  return inputState;
}

//==========================
//SuperGameBoy::Construction
//==========================

SuperGameBoy::SuperGameBoy() : gambatte_(0) {
  romdata = ramdata = rtcdata = 0;
  romsize = ramsize = rtcsize = 0;
  buffer = new uint32_t[160 * 144];
  
  op_step = op_step_default;
  op_call = op_step_default;
  op_ret  = op_step_default;
  op_irq  = op_step_default;
  op_read = op_read_default;
  op_readpc = op_read_default;
  op_write = op_write_default;
}

SuperGameBoy::~SuperGameBoy() {
  delete[] buffer;
}
