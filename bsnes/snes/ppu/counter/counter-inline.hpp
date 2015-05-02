//this should only be called by CPU::PPUcounter::tick();
//keeps track of previous counter positions in history table
void PPUcounter::tick() {
  status.hcounter += 2;  //increment by smallest unit of time
  if(status.hcounter >= 1360 && status.hcounter == lineclocks()) {
    status.hcounter = 0;
    vcounter_tick();
  }

  history.index = (history.index + 1) & History::Mask;
  history.vcounter[history.index] = status.vcounter;
  history.hcounter[history.index] = status.hcounter;
}

//this should only be called by PPU::PPUcounter::tick(n);
//allows stepping by more than the smallest unit of time
void PPUcounter::tick(unsigned clocks) {
  status.hcounter += clocks;
  if(status.hcounter >= lineclocks()) {
    status.hcounter -= lineclocks();
    vcounter_tick();
  }
}

//internal
void PPUcounter::vcounter_tick() {
  if(++status.vcounter == 128) {
    status.interlace = ppu.interlace();
    status.fieldlines = (system.region() == System::Region::NTSC) ? 262 : 312;
    if(status.interlace && !status.field) ++status.fieldlines;
  } else if(status.vcounter == status.fieldlines) {
    status.vcounter = 0;
    status.field = !status.field;
  }
  if(scanline) scanline();
}

bool   PPUcounter::field     () const { return status.field; }
uint16 PPUcounter::vcounter  () const { return status.vcounter; }
uint16 PPUcounter::hcounter  () const { return status.hcounter; }
uint16 PPUcounter::fieldlines() const { return status.fieldlines; }

uint16 PPUcounter::vcounter(unsigned offset) const { return history.vcounter[(history.index - (offset >> 1)) & History::Mask]; }
uint16 PPUcounter::hcounter(unsigned offset) const { return history.hcounter[(history.index - (offset >> 1)) & History::Mask]; }

//one PPU dot = 4 CPU clocks
//
//PPU dots 323 and 327 are 6 CPU clocks long.
//this does not apply to NTSC non-interlace scanline 240 on odd fields. this is
//because the PPU skips one dot to alter the color burst phase of the video signal.
//
//dot 323 range = { 1292, 1294, 1296 }
//dot 327 range = { 1310, 1312, 1314 }

uint16 PPUcounter::hdot() const {
  if(system.region() == System::Region::NTSC && status.interlace == false && vcounter() == 240 && field() == 1) {
    return (hcounter() >> 2);
  } else {
    return (hcounter() - ((hcounter() > 1292) << 1) - ((hcounter() > 1310) << 1)) >> 2;
  }
}

uint16 PPUcounter::lineclocks() const {
  if(system.region() == System::Region::NTSC) {
    if(status.interlace == false && vcounter() == 240 && field()) return 1360;
  } else {
    if(status.interlace == true  && vcounter() == 311 && field()) return 1368;
  }
  return 1364;
}

void PPUcounter::reset() {
  status.hcounter  = 0;
  status.vcounter  = 0;
  status.fieldlines= (system.region() == System::Region::NTSC) ? 262 : 312;
  status.interlace = false;
  status.field     = 0;

  history.index    = 0;

  for(unsigned i = 0; i < History::Depth; i++) {
    history.vcounter[i] = 0;
    history.hcounter[i] = 0;
  }
}
