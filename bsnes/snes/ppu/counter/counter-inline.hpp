//this should only be called by CPU::PPUcounter::tick();
//keeps track of previous counter positions in history table
void PPUcounter::tick() {
  status.hcounter += 2;  //increment by smallest unit of time
  if(status.hcounter == lineclocks()) {
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
  ++status.vcounter;
  status.lineclocks = 1364;

  if(vcounter() == 128) {
    //synchronize with PPU interlace state
    //fieldlines[0] = lines in even field: (262 or 312)+1 if interlace
    //fieldlines[1] = lines in odd field: always 262 or 312
    //note that fieldlines[0] uniquely identifies both region and interlace mode
    //(262 = NTSC non-interlace, 313 = PAL interlace)
    status.fieldlines[0] = status.fieldlines[1] + ppu.interlace();
  } else if(vcounter() == 240 && status.fieldlines[0] == 262 && field()) {
    //line 240 of NTSC non-interlace odd fields is 4 clocks shorter (has no long dots)
    status.lineclocks -= 4;
  } else if(vcounter() == 311 && status.fieldlines[0] == 313 && field()) {
    //line 311 of PAL interlace odd fields is 4 clocks longer (has one extra dot)
    status.lineclocks += 4;
  } else if(vcounter() == status.fieldlines[status.field]) {
    status.vcounter = 0;
    status.field = !status.field;
  }
  if(scanline) scanline();
}

bool   PPUcounter::field     () const { return status.field; }
uint16 PPUcounter::vcounter  () const { return status.vcounter; }
uint16 PPUcounter::hcounter  () const { return status.hcounter; }
uint16 PPUcounter::lineclocks() const { return status.lineclocks; }
uint16 PPUcounter::fieldlines() const { return status.fieldlines[status.field]; }

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
  if(lineclocks() == 1360) {
    return (hcounter() >> 2);
  } else {
    return (hcounter() - ((hcounter() > 1292) << 1) - ((hcounter() > 1310) << 1)) >> 2;
  }
}

void PPUcounter::reset() {
  status.hcounter      = 0;
  status.vcounter      = 0;
  status.lineclocks    = 1364;
  status.fieldlines[0] = status.fieldlines[1] = (system.region() == System::Region::NTSC ? 262 : 312);
  status.field         = 0;

  history.index        = 0;

  for(unsigned i = 0; i < History::Depth; i++) {
    history.vcounter[i] = 0;
    history.hcounter[i] = 0;
  }
}
