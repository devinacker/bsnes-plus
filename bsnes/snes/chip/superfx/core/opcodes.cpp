#ifdef SUPERFX_CPP

//$00 stop
void SuperFX::op_stop() {
  if(regs.cfgr.irq == 0) {
    regs.sfr.irq = 1;
    cpu.regs.irq = 1;
  }

  regs.sfr.g = 0;
  regs.pipeline = 0x01;
  regs.reset();
}

//$01 nop
void SuperFX::op_nop() {
  regs.reset();
}

//$02 cache
void SuperFX::op_cache() {
  if(regs.cbr != (regs.r[15] & 0xfff0)) {
    regs.cbr = regs.r[15] & 0xfff0;
    cache_flush();
  }
  regs.reset();
}

//$03 lsr
void SuperFX::op_lsr() {
  regs.sfr.cy = (regs.sr() & 1);
  regs.dr() = regs.sr() >> 1;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$04 rol
void SuperFX::op_rol() {
  bool carry = (regs.sr() & 0x8000);
  regs.dr() = (regs.sr() << 1) | regs.sfr.cy;
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = carry;
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
}

//$05 bra e
void SuperFX::op_bra() {
  regs.r[15] += (int8)pipe();
}

//$06 bge e
void SuperFX::op_bge() {
  int8 e = pipe();
  if(regs.sfr.s == regs.sfr.ov) regs.r[15] += e;
}

//$07 blt e
void SuperFX::op_blt() {
  int8 e = pipe();
  if(regs.sfr.s != regs.sfr.ov) regs.r[15] += e;
}

//$08 bne e
void SuperFX::op_bne() {
  int8 e = pipe();
  if(!regs.sfr.z) regs.r[15] += e;
}

//$09 beq e
void SuperFX::op_beq() {
  int8 e = pipe();
  if(regs.sfr.z) regs.r[15] += e;
}

//$0a bpl e
void SuperFX::op_bpl() {
  int8 e = pipe();
  if(!regs.sfr.s) regs.r[15] += e;
}

//$0b bmi e
void SuperFX::op_bmi() {
  int8 e = pipe();
  if(regs.sfr.s) regs.r[15] += e;
}

//$0c bcc e
void SuperFX::op_bcc() {
  int8 e = pipe();
  if(!regs.sfr.cy) regs.r[15] += e;
}

//$0d bcs e
void SuperFX::op_bcs() {
  int8 e = pipe();
  if(regs.sfr.cy) regs.r[15] += e;
}

//$0e bvc e
void SuperFX::op_bvc() {
  int8 e = pipe();
  if(!regs.sfr.ov) regs.r[15] += e;
}

//$0f bvs e
void SuperFX::op_bvs() {
  int8 e = pipe();
  if(regs.sfr.ov) regs.r[15] += e;
}

//$10-1f(b0): to rN
//$10-1f(b1): move rN
void SuperFX::op_to_move(unsigned n) {
  if(regs.sfr.b == 0) {
    regs.dreg = n;
  } else {
    regs.r[n] = regs.sr();
    regs.reset();
  }
}

//$20-2f: with rN
void SuperFX::op_with(unsigned n) {
  regs.sreg = n;
  regs.dreg = n;
  regs.sfr.b = 1;
}

//$30-3b(alt0): stw (rN)
//$30-3b(alt1): stb (rN)
void SuperFX::op_stw_stb(unsigned n) {
  regs.ramaddr = regs.r[n];
  rambuffer_write(regs.ramaddr, regs.sr());
  if(!regs.sfr.alt1)
    rambuffer_write(regs.ramaddr ^ 1, regs.sr() >> 8);
  regs.reset();
}

//$3c loop
void SuperFX::op_loop() {
  regs.r[12]--;
  regs.sfr.s = (regs.r[12] & 0x8000);
  regs.sfr.z = (regs.r[12] == 0);
  if(!regs.sfr.z) regs.r[15] = regs.r[13];
  regs.reset();
}

//$3d alt1
void SuperFX::op_alt1() {
  regs.sfr.b = 0;
  regs.sfr.alt1 = 1;
}

//$3e alt2
void SuperFX::op_alt2() {
  regs.sfr.b = 0;
  regs.sfr.alt2 = 1;
}

//$3f alt3
void SuperFX::op_alt3() {
  regs.sfr.b = 0;
  regs.sfr.alt1 = 1;
  regs.sfr.alt2 = 1;
}

//$40-4b(alt0): ldw (rN)
//$40-4b(alt1): ldb (rN)
void SuperFX::op_ldw_ldb(unsigned n) {
  regs.ramaddr = regs.r[n];
  unsigned data = rambuffer_read(regs.ramaddr);
  if(!regs.sfr.alt1)
    data |= rambuffer_read(regs.ramaddr ^ 1) << 8;
  regs.dr() = data;
  regs.reset();
}

//$4c(alt0): plot
void SuperFX::op_plot() {
  plot(regs.r[1], regs.r[2]);
  regs.r[1]++;
  regs.reset();
}

//$4c(alt1): rpix
void SuperFX::op_rpix() {
  regs.dr() = rpix(regs.r[1], regs.r[2]);
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$4d: swap
void SuperFX::op_swap() {
  regs.dr() = (regs.sr() >> 8) | (regs.sr() << 8);
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$4e(alt0): color
void SuperFX::op_color() {
  regs.colr = color(regs.sr());
  regs.reset();
}

//$4e(alt1): cmode
void SuperFX::op_cmode() {
  regs.por = regs.sr();
  regs.reset();
}

//$4f: not
void SuperFX::op_not() {
  regs.dr() = ~regs.sr();
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

unsigned SuperFX::reg_or_imm(unsigned n) {
  return regs.sfr.alt2 ? n : regs.r[n];
}

//$50-5f(alt0): add rN
//$50-5f(alt1): adc rN
//$50-5f(alt2): add #N
//$50-5f(alt3): adc #N
void SuperFX::op_add_adc(unsigned n) {
  unsigned b = reg_or_imm(n);
  unsigned r = regs.sr() + b + (regs.sfr.alt1 & regs.sfr.cy);
  regs.sfr.ov = (regs.sr() ^ r) & (b ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0x10000);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$60-6f(alt0): sub rN
//$60-6f(alt1): sbc rN
//$60-6f(alt2): sub #N
void SuperFX::op_sub_sbc(unsigned n) {
  int b = reg_or_imm(n);
  int r = regs.sr() - b - (regs.sfr.alt1 & !regs.sfr.cy);
  regs.sfr.ov = (regs.sr() ^ b) & (regs.sr() ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.dr() = r;
  regs.reset();
}

//$60-6f(alt3): cmp rN
void SuperFX::op_cmp(unsigned n) {
  int b = regs.r[n];
  int r = regs.sr() - b;
  regs.sfr.ov = (regs.sr() ^ b) & (regs.sr() ^ r) & 0x8000;
  regs.sfr.s  = (r & 0x8000);
  regs.sfr.cy = (r >= 0);
  regs.sfr.z  = ((uint16_t)r == 0);
  regs.reset();
}

//$70: merge
void SuperFX::op_merge() {
  regs.dr() = (regs.r[7] & 0xff00) | (regs.r[8] >> 8);
  regs.sfr.ov = (regs.dr() & 0xc0c0);
  regs.sfr.s  = (regs.dr() & 0x8080);
  regs.sfr.cy = (regs.dr() & 0xe0e0);
  regs.sfr.z  = (regs.dr() & 0xf0f0);
  regs.reset();
}

//$71-7f(alt0): and rN
//$71-7f(alt1): bic rN
//$71-7f(alt2): and #N
//$71-7f(alt3): bic #N
void SuperFX::op_and_bic(unsigned n) {
  unsigned b = reg_or_imm(n);
  if(regs.sfr.alt1) b = ~b;
  regs.dr() = regs.sr() & b;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$80-8f(alt0): mult rN
//$80-8f(alt2): mult #N
void SuperFX::op_mult(unsigned n) {
  int8 b = reg_or_imm(n);
  regs.dr() = (int8)regs.sr() * b;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
  if(!regs.cfgr.ms0) add_clocks(regs.clsr.divider);
}

//$80-8f(alt1): umult rN
//$80-8f(alt3): umult #N
void SuperFX::op_umult(unsigned n) {
  uint8 b = reg_or_imm(n);
  regs.dr() = (uint8)regs.sr() * b;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
  if(!regs.cfgr.ms0) add_clocks(regs.clsr.divider);
}

//$90: sbk
void SuperFX::op_sbk() {
  rambuffer_write(regs.ramaddr ^ 0, regs.sr() >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.sr() >> 8);
  regs.reset();
}

//$91-94: link #N
void SuperFX::op_link(unsigned n) {
  regs.r[11] = regs.r[15] + n;
  regs.reset();
}

//$95: sex
void SuperFX::op_sex() {
  regs.dr() = (int8)regs.sr();
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$96(alt0): asr
//$96(alt1): div2
void SuperFX::op_asr_div2() {
  int16 r = regs.sr();
  regs.sfr.cy = (r & 1);
  regs.dr() = (r == -1 && regs.sfr.alt1) ? 0 : r >> 1;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$97: ror
void SuperFX::op_ror() {
  bool carry = (regs.sr() & 1);
  regs.dr() = (regs.sfr.cy << 15) | (regs.sr() >> 1);
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = carry;
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
}

//$98-9d(alt0): jmp rN
void SuperFX::op_jmp(unsigned n) {
  regs.r[15] = regs.r[n];
  regs.reset();
}

//$98-9d(alt1): ljmp rN
void SuperFX::op_ljmp(unsigned n) {
  regs.pbr = regs.r[n] & 0x7f;
  regs.r[15] = regs.sr();
  regs.cbr = regs.r[15] & 0xfff0;
  cache_flush();
  regs.reset();
}

//$9e: lob
void SuperFX::op_lob() {
  regs.dr() = regs.sr() & 0xff;
  regs.sfr.s = (regs.dr() & 0x80);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$9f(alt0): fmult
//$9f(alt1): lmult
void SuperFX::op_fmult_lmult() {
  int result = (int16_t)regs.sr() * (int16_t)regs.r[6];
  if(regs.sfr.alt1) regs.r[4] = result;
  regs.dr() = result >> 16;
  regs.sfr.s  = (regs.dr() & 0x8000);
  regs.sfr.cy = (result & 0x8000);
  regs.sfr.z  = (regs.dr() == 0);
  regs.reset();
  add_clocks((regs.cfgr.ms0 ? 3 : 7) * regs.clsr.divider);
}

//$a0-af(alt0): ibt rN,#pp
void SuperFX::op_ibt(unsigned n) {
  regs.r[n] = (int8)pipe();
  regs.reset();
}

//$a0-af(alt1): lms rN,(yy)
void SuperFX::op_lms(unsigned n) {
  regs.ramaddr = pipe() << 1;
  unsigned data = rambuffer_read(regs.ramaddr);
  data |= rambuffer_read(regs.ramaddr ^ 1) << 8;
  regs.r[n] = data;
  regs.reset();
}

//$a0-af(alt2): sms (yy),rN
void SuperFX::op_sms(unsigned n) {
  regs.ramaddr = pipe() << 1;
  rambuffer_write(regs.ramaddr ^ 0, regs.r[n] >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.r[n] >> 8);
  regs.reset();
}

//$b0-bf(b0): from rN
//$b0-bf(b1): moves rN
void SuperFX::op_from_moves(unsigned n) {
  if(regs.sfr.b == 0) {
    regs.sreg = n;
  } else {
    regs.dr() = regs.r[n];
    regs.sfr.ov = (regs.dr() & 0x80);
    regs.sfr.s  = (regs.dr() & 0x8000);
    regs.sfr.z  = (regs.dr() == 0);
    regs.reset();
  }
}

//$c0: hib
void SuperFX::op_hib() {
  regs.dr() = regs.sr() >> 8;
  regs.sfr.s = (regs.dr() & 0x80);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$c1-cf(alt0): or rN
//$c1-cf(alt2): or #N
void SuperFX::op_or(unsigned n) {
  unsigned b = reg_or_imm(n);
  regs.dr() = regs.sr() | b;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$c1-cf(alt1): xor rN
//$c1-cf(alt3): xor #N
void SuperFX::op_xor(unsigned n) {
  unsigned b = reg_or_imm(n);
  regs.dr() = regs.sr() ^ b;
  regs.sfr.s = (regs.dr() & 0x8000);
  regs.sfr.z = (regs.dr() == 0);
  regs.reset();
}

//$d0-de: inc rN
void SuperFX::op_inc(unsigned n) {
  regs.r[n]++;
  regs.sfr.s = (regs.r[n] & 0x8000);
  regs.sfr.z = (regs.r[n] == 0);
  regs.reset();
}

//$df(alt0): getc
void SuperFX::op_getc() {
  regs.colr = color(rombuffer_read());
  regs.reset();
}

//$df(alt2): ramb
void SuperFX::op_ramb() {
  rambuffer_sync();
  regs.rambr = regs.sr() & 1;
  regs.reset();
}

//$df(alt3): romb
void SuperFX::op_romb() {
  rombuffer_sync();
  regs.rombr = regs.sr() & 0x7f;
  regs.reset();
}

//$e0-ee: dec rN
void SuperFX::op_dec(unsigned n) {
  regs.r[n]--;
  regs.sfr.s = (regs.r[n] & 0x8000);
  regs.sfr.z = (regs.r[n] == 0);
  regs.reset();
}

//$ef(alt0): getb
void SuperFX::op_getb() {
  regs.dr() = rombuffer_read();
  regs.reset();
}

//$ef(alt1): getbh
void SuperFX::op_getbh() {
  regs.dr() = (rombuffer_read() << 8) | (regs.sr() & 0x00ff);
  regs.reset();
}

//$ef(alt2): getbl
void SuperFX::op_getbl() {
  regs.dr() = (regs.sr() & 0xff00) | (rombuffer_read() << 0);
  regs.reset();
}

//$ef(alt3): getbs
void SuperFX::op_getbs() {
  regs.dr() = (int8)rombuffer_read();
  regs.reset();
}

//$f0-ff(alt0): iwt rN,#xx
void SuperFX::op_iwt(unsigned n) {
  unsigned data = pipe();
  data |= pipe() << 8;
  regs.r[n] = data;
  regs.reset();
}

//$f0-ff(alt1): lm rN,(xx)
void SuperFX::op_lm(unsigned n) {
  regs.ramaddr  = pipe() << 0;
  regs.ramaddr |= pipe() << 8;
  unsigned data = rambuffer_read(regs.ramaddr);
  data |= rambuffer_read(regs.ramaddr ^ 1) << 8;
  regs.r[n] = data;
  regs.reset();
}

//$f0-ff(alt2): sm (xx),rN
void SuperFX::op_sm(unsigned n) {
  regs.ramaddr  = pipe() << 0;
  regs.ramaddr |= pipe() << 8;
  rambuffer_write(regs.ramaddr ^ 0, regs.r[n] >> 0);
  rambuffer_write(regs.ramaddr ^ 1, regs.r[n] >> 8);
  regs.reset();
}

#endif
