#ifdef PPU_CPP

void PPU::Screen::scanline() {
  output = self.output + self.vcounter() * 1024;
  if(self.display.interlace && self.field()) output += 512;

  //the first hires pixel of each scanline is transparent
  //note: exact value initializations are not confirmed on hardware
  math.main.color = math.sub.color = get_color(0);
  math.main.color_enable = !(self.window.regs.col_main_mask & 1);
  math.sub.color_enable = !(self.window.regs.col_sub_mask & 1) && regs.back_color_enable;

  math.transparent = true;
  math.addsub_mode = false;
  math.color_halve = regs.color_halve && !regs.addsub_mode && math.main.color_enable;
}

void PPU::Screen::run() {
  if(ppu.vcounter() == 0) return;

  bool hires = self.regs.pseudo_hires || self.regs.bgmode == 5 || self.regs.bgmode == 6;
  uint16 sscolor = get_pixel_sub(hires);
  uint16 mscolor = get_pixel_main();
  *output++ = light_table[self.regs.display_brightness][hires ? sscolor : mscolor];
  *output++ = light_table[self.regs.display_brightness][mscolor];
}

uint16 PPU::Screen::get_pixel_sub(bool hires) {
  if(self.regs.display_disable) return 0;

  unsigned priority = 0;
  if(self.bg1.output.sub.priority) {
    priority = self.bg1.output.sub.priority;
    if(regs.direct_color && (self.regs.bgmode == 3 || self.regs.bgmode == 4 || self.regs.bgmode == 7)) {
      math.sub.color = get_direct_color(self.bg1.output.sub.palette, self.bg1.output.sub.tile);
    } else {
      math.sub.color = get_color(self.bg1.output.sub.palette);
    }
  }
  if(self.bg2.output.sub.priority > priority) {
    priority = self.bg2.output.sub.priority;
    math.sub.color = get_color(self.bg2.output.sub.palette);
  }
  if(self.bg3.output.sub.priority > priority) {
    priority = self.bg3.output.sub.priority;
    math.sub.color = get_color(self.bg3.output.sub.palette);
  }
  if(self.bg4.output.sub.priority > priority) {
    priority = self.bg4.output.sub.priority;
    math.sub.color = get_color(self.bg4.output.sub.palette);
  }
  if(self.oam.output.sub.priority > priority) {
    priority = self.oam.output.sub.priority;
    math.sub.color = get_color(self.oam.output.sub.palette);
  }
  if(math.transparent = (priority == 0)) math.sub.color = get_color(0);

  if(!hires) return 0;
  if(!math.sub.color_enable) return math.main.color_enable ? math.sub.color : 0;
  return addsub(math.main.color_enable ? math.sub.color : 0,
                math.addsub_mode ? math.main.color : regs.color);
}

uint16 PPU::Screen::get_pixel_main() {
  if(self.regs.display_disable) return 0;

  unsigned priority = 0;
  if(self.bg1.output.main.priority) {
    priority = self.bg1.output.main.priority;
    if(regs.direct_color && (self.regs.bgmode == 3 || self.regs.bgmode == 4 || self.regs.bgmode == 7)) {
      math.main.color = get_direct_color(self.bg1.output.main.palette, self.bg1.output.main.tile);
    } else {
      math.main.color = get_color(self.bg1.output.main.palette);
    }
    math.sub.color_enable = regs.bg1_color_enable;
  }
  if(self.bg2.output.main.priority > priority) {
    priority = self.bg2.output.main.priority;
    math.main.color = get_color(self.bg2.output.main.palette);
    math.sub.color_enable = regs.bg2_color_enable;
  }
  if(self.bg3.output.main.priority > priority) {
    priority = self.bg3.output.main.priority;
    math.main.color = get_color(self.bg3.output.main.palette);
    math.sub.color_enable = regs.bg3_color_enable;
  }
  if(self.bg4.output.main.priority > priority) {
    priority = self.bg4.output.main.priority;
    math.main.color = get_color(self.bg4.output.main.palette);
    math.sub.color_enable = regs.bg4_color_enable;
  }
  if(self.oam.output.main.priority > priority) {
    priority = self.oam.output.main.priority;
    math.main.color = get_color(self.oam.output.main.palette);
    math.sub.color_enable = (regs.oam_color_enable && self.oam.output.main.palette >= 192);
  }
  if(priority == 0) {
    math.main.color = get_color(0);
    math.sub.color_enable = regs.back_color_enable;
  }

  if(!self.window.output.sub.color_enable) math.sub.color_enable = false;
  math.main.color_enable = self.window.output.main.color_enable;
  if(!math.sub.color_enable) return math.main.color_enable ? math.main.color : 0;

  if(regs.addsub_mode && math.transparent) {
    math.addsub_mode = false;
    math.color_halve = false;
  } else {
    math.addsub_mode = regs.addsub_mode;
    math.color_halve = regs.color_halve && math.main.color_enable;
  }

  return addsub(math.main.color_enable ? math.main.color : 0,
                math.addsub_mode ? math.sub.color : regs.color);
}

uint16 PPU::Screen::addsub(unsigned x, unsigned y) {
  if(!regs.color_mode) {
    if(!math.color_halve) {
      unsigned sum = x + y;
      unsigned carry = (sum - ((x ^ y) & 0x0421)) & 0x8420;
      return (sum - carry) | (carry - (carry >> 5));
    } else {
      return (x + y - ((x ^ y) & 0x0421)) >> 1;
    }
  } else {
    unsigned diff = x - y + 0x8420;
    unsigned borrow = (diff - ((x ^ y) & 0x8420)) & 0x8420;
    if(!math.color_halve) {
      return   (diff - borrow) & (borrow - (borrow >> 5));
    } else {
      return (((diff - borrow) & (borrow - (borrow >> 5))) & 0x7bde) >> 1;
    }
  }
}

uint16 PPU::Screen::get_color(unsigned palette) {
  palette <<= 1;
  self.regs.cgram_iaddr = palette;
  return memory::cgram[palette + 0] + (memory::cgram[palette + 1] << 8);
}

uint16 PPU::Screen::get_direct_color(unsigned palette, unsigned tile) {
  //palette = -------- BBGGGRRR
  //tile    = ---bgr-- --------
  //output  = 0BBb00GG Gg0RRRr0
  return ((palette << 7) & 0x6000) + ((tile >> 0) & 0x1000)
       + ((palette << 4) & 0x0380) + ((tile >> 5) & 0x0040)
       + ((palette << 2) & 0x001c) + ((tile >> 9) & 0x0002);
}

void PPU::Screen::reset() {
  regs.addsub_mode = 0;
  regs.direct_color = 0;
  regs.color_mode = 0;
  regs.color_halve = 0;
  regs.bg1_color_enable = 0;
  regs.bg2_color_enable = 0;
  regs.bg3_color_enable = 0;
  regs.bg4_color_enable = 0;
  regs.oam_color_enable = 0;
  regs.back_color_enable = 0;
  regs.color = 0;

  math.main.color = 0;
  math.sub.color = 0;
  math.transparent = false;
  math.main.color_enable = false;
  math.sub.color_enable = false;
  math.addsub_mode = false;
  math.color_halve = false;
}

PPU::Screen::Screen(PPU &self) : self(self) {
  for(unsigned l = 0; l < 16; l++) {
    for(unsigned r = 0; r < 32; r++) {
      for(unsigned g = 0; g < 32; g++) {
        for(unsigned b = 0; b < 32; b++) {
          double luma = (double)l / 15.0;
          unsigned ar = (luma * r + 0.5);
          unsigned ag = (luma * g + 0.5);
          unsigned ab = (luma * b + 0.5);
          light_table[l][(r << 10) + (g << 5) + b] = (ab << 10) + (ag << 5) + ar;
        }
      }
    }
  }
}

#endif
