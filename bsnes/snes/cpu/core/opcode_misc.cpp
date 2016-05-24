#ifdef CPUCORE_CPP

void CPUcore::op_nop() {
L op_io_irq();
}

void CPUcore::op_wdm() {
L op_readpc();
}

void CPUcore::op_xba() {
  op_io();
L op_io();
  std::swap(regs.a.l, regs.a.h);
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

template<int adjust> void CPUcore::op_move_b() {
  dp = op_readpc();
  sp = op_readpc();
  regs.db = dp;
  rd.l = op_readlong((sp << 16) | regs.x.w);
  op_writelong((dp << 16) | regs.y.w, rd.l);
  op_io();
  regs.x.l += adjust;
  regs.y.l += adjust;
L op_io();
  if(regs.a.w--) regs.pc.w -= 3;
}

template<int adjust> void CPUcore::op_move_w() {
  dp = op_readpc();
  sp = op_readpc();
  regs.db = dp;
  rd.l = op_readlong((sp << 16) | regs.x.w);
  op_writelong((dp << 16) | regs.y.w, rd.l);
  op_io();
  regs.x.w += adjust;
  regs.y.w += adjust;
L op_io();
  if(regs.a.w--) regs.pc.w -= 3;
}

template<int vectorE, int vectorN> void CPUcore::op_interrupt() {
  int vector = regs.e ? vectorE : vectorN;
  op_readpc();
  if(!regs.e) op_writestack(regs.pc.b);
  op_writestack(regs.pc.h);
  op_writestack(regs.pc.l);
  op_writestack(regs.p);
  rd.l = op_readlong(vector + 0);
  regs.pc.b = 0;
  regs.p.i = 1;
  regs.p.d = 0;
L rd.h = op_readlong(vector + 1);
  regs.pc.w = rd.w;
}

void CPUcore::op_stp() {
  while(regs.wai = true) {
L   op_io();
  }
}

void CPUcore::op_wai() {
  regs.wai = true;
  while(regs.wai) {
L   op_io();
  }
  op_io();
}

void CPUcore::op_xce() {
L op_io_irq();
  bool carry = regs.p.c;
  regs.p.c = regs.e;
  regs.e = carry;
  if(regs.e) {
    regs.p |= 0x30;
    regs.s.h = 0x01;
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

void CPUcore::op_clc() {
L op_io_irq();
  regs.p.c = 0;
}

void CPUcore::op_sec() {
L op_io_irq();
  regs.p.c = 1;
}

void CPUcore::op_cli() {
L op_io_irq();
  regs.p.i = 0;
}

void CPUcore::op_sei() {
L op_io_irq();
  regs.p.i = 1;
}

void CPUcore::op_clv() {
L op_io_irq();
  regs.p.v = 0;
}

void CPUcore::op_cld() {
L op_io_irq();
  regs.p.d = 0;
}

void CPUcore::op_sed() {
L op_io_irq();
  regs.p.d = 1;
}

void CPUcore::op_rep() {
  rd.l = ~op_readpc();
  if(regs.e) rd.l |= 0x30;
L op_io();
  regs.p &= rd.l;
  update_table();
}

void CPUcore::op_sep() {
  rd.l = op_readpc();
L op_io();
  regs.p |= rd.l;
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

template<int from, int to> void CPUcore::op_transfer_b() {
L op_io_irq();
  regs.r[to].l = regs.r[from].l;
  regs.p.n = (regs.r[to].l & 0x80);
  regs.p.z = (regs.r[to].l == 0);
}

template<int from, int to> void CPUcore::op_transfer_w() {
L op_io_irq();
  regs.r[to].w = regs.r[from].w;
  regs.p.n = (regs.r[to].w & 0x8000);
  regs.p.z = (regs.r[to].w == 0);
}

void CPUcore::op_tcs() {
L op_io_irq();
  regs.s.w = regs.a.w;
  if(regs.e) regs.s.h = 0x01;
}

void CPUcore::op_tsx_b() {
L op_io_irq();
  regs.x.l = regs.s.l;
  regs.p.n = (regs.x.l & 0x80);
  regs.p.z = (regs.x.l == 0);
}

void CPUcore::op_tsx_w() {
L op_io_irq();
  regs.x.w = regs.s.w;
  regs.p.n = (regs.x.w & 0x8000);
  regs.p.z = (regs.x.w == 0);
}

void CPUcore::op_txs() {
L op_io_irq();
  regs.s.w = regs.x.w;
  if(regs.e) regs.s.h = 0x01;
}

template<int n> void CPUcore::op_push_b() {
  op_io();
L op_writestack(regs.r[n].l);
}

template<int n> void CPUcore::op_push_w() {
  op_io();
  op_writestack(regs.r[n].h);
L op_writestack(regs.r[n].l);
}

void CPUcore::op_phd() {
  op_io();
  op_writestackn(regs.d.h);
L op_writestackn(regs.d.l);
  if(regs.e) regs.s.h = 0x01;
}

void CPUcore::op_phb() {
  op_io();
L op_writestack(regs.db);
}

void CPUcore::op_phk() {
  op_io();
L op_writestack(regs.pc.b);
}

void CPUcore::op_php() {
  op_io();
L op_writestack(regs.p);
}

template<int n> void CPUcore::op_pull_b() {
  op_io();
  op_io();
L regs.r[n].l = op_readstack();
  regs.p.n = (regs.r[n].l & 0x80);
  regs.p.z = (regs.r[n].l == 0);
}

template<int n> void CPUcore::op_pull_w() {
  op_io();
  op_io();
  regs.r[n].l = op_readstack();
L regs.r[n].h = op_readstack();
  regs.p.n = (regs.r[n].w & 0x8000);
  regs.p.z = (regs.r[n].w == 0);
}

void CPUcore::op_pld() {
  op_io();
  op_io();
  regs.d.l = op_readstackn();
L regs.d.h = op_readstackn();
  regs.p.n = (regs.d.w & 0x8000);
  regs.p.z = (regs.d.w == 0);
  if(regs.e) regs.s.h = 0x01;
}

void CPUcore::op_plb() {
  op_io();
  op_io();
L regs.db = op_readstack();
  regs.p.n = (regs.db & 0x80);
  regs.p.z = (regs.db == 0);
}

void CPUcore::op_plp() {
  op_io();
  op_io();
L regs.p = op_readstack() | (regs.e ? 0x30 : 0);
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

void CPUcore::op_pea() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_writestackn(aa.h);
L op_writestackn(aa.l);
  if(regs.e) regs.s.h = 0x01;
}

void CPUcore::op_pei() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdpn(dp + 0);
  aa.h = op_readdpn(dp + 1);
  op_writestackn(aa.h);
L op_writestackn(aa.l);
  if(regs.e) regs.s.h = 0x01;
}

void CPUcore::op_per() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  rd.w = regs.pc.d + (int16)aa.w;
  op_writestackn(rd.h);
L op_writestackn(rd.l);
  if(regs.e) regs.s.h = 0x01;
}

#endif
