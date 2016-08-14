#ifdef SUPERFX_CPP

void SuperFX::op_exec(uint8 opcode) {
  switch(opcode) {

#define op(id, name)   case id: return op_##name();

#define op4(id, name)  case id+ 0: case id+ 1: case id+ 2: case id+ 3: return op_##name(opcode & 15);

#define op12(id, name) case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
                       case id+ 8: case id+ 9: case id+10: case id+11: return op_##name(opcode & 15);

#define op15(id, name) case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
                       case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: \
                       return op_##name(opcode & 15);

#define op16(id, name) case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
                       case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: case id+15: \
                       return op_##name(opcode & 15);

#define opalt1(id, name, name1) case id: return (!regs.sfr.alt1) ? op_##name() : op_##name1();

#define op6alt1(id, name, name1) case id+0: case id+1: case id+2: case id+3: case id+4: case id+5: \
                                 return (!regs.sfr.alt1) ? op_##name(opcode & 15) : op_##name1(opcode & 15);

#define op15a1(id, name, name1) case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
                                case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: \
                                return (!regs.sfr.alt1) ? op_##name(opcode & 15) : op_##name1(opcode & 15);

#define op16a1(id, name, name1) case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
                                case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: case id+15: \
                                return (!regs.sfr.alt1) ? op_##name(opcode & 15) : op_##name1(opcode & 15);

#define op16a3(id, name, name3) case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
                                case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: case id+15: \
                                return (regs.sfr.alt1 & regs.sfr.alt2) ? op_##name3(opcode & 15) : op_##name(opcode & 15);

#define op16a12(id, name, name1, name2) \
  case id+ 0: case id+ 1: case id+ 2: case id+ 3: case id+ 4: case id+ 5: case id+ 6: case id+ 7: \
  case id+ 8: case id+ 9: case id+10: case id+11: case id+12: case id+13: case id+14: case id+15: \
  return regs.sfr.alt1 ? op_##name1(opcode & 15) : \
         regs.sfr.alt2 ? op_##name2(opcode & 15) : op_##name(opcode & 15);

#define opalt23(id, name, name2, name3) case id: \
  return (!regs.sfr.alt2) ? op_##name() : \
         (!regs.sfr.alt1) ? op_##name2() : op_##name3();

#define opa123(id, name, name1, name2, name3) case id: \
  return (!regs.sfr.alt2) ? ((!regs.sfr.alt1) ? op_##name()  : op_##name1()) : \
                            ((!regs.sfr.alt1) ? op_##name2() : op_##name3());

#define opb(id, cond) case id: return (cond) ? op_bra() : op_nobranch();

#define bge (regs.sfr.s == regs.sfr.ov)
#define blt (regs.sfr.s != regs.sfr.ov)
#define bne !regs.sfr.z
#define beq regs.sfr.z
#define bpl !regs.sfr.s
#define bmi regs.sfr.s
#define bcc !regs.sfr.cy
#define bcs regs.sfr.cy
#define bvc !regs.sfr.ov
#define bvs regs.sfr.ov

op     (0x00, stop)
op     (0x01, nop)
op     (0x02, cache)
op     (0x03, lsr)
op     (0x04, rol)
op     (0x05, bra)
opb    (0x06, bge)
opb    (0x07, blt)
opb    (0x08, bne)
opb    (0x09, beq)
opb    (0x0a, bpl)
opb    (0x0b, bmi)
opb    (0x0c, bcc)
opb    (0x0d, bcs)
opb    (0x0e, bvc)
opb    (0x0f, bvs)
op16   (0x10, to_move)
op16   (0x20, with)
op12   (0x30, stw_stb)
op     (0x3c, loop)
op     (0x3d, alt1)
op     (0x3e, alt2)
op     (0x3f, alt3)
op12   (0x40, ldw_ldb)
opalt1 (0x4c, plot, rpix)
op     (0x4d, swap)
opalt1 (0x4e, color, cmode)
op     (0x4f, not)
op16   (0x50, add_adc)
op16a3 (0x60, sub_sbc, cmp)
op     (0x70, merge)
op15   (0x71, and_bic)
op16a1 (0x80, mult, umult)
op     (0x90, sbk)
op4    (0x91, link)
op     (0x95, sex)
op     (0x96, asr_div2)
op     (0x97, ror)
op6alt1(0x98, jmp, ljmp)
op     (0x9e, lob)
op     (0x9f, fmult_lmult)
op16a12(0xa0, ibt, lms, sms)
op16   (0xb0, from_moves)
op     (0xc0, hib)
op15a1 (0xc1, or, xor)
op15   (0xd0, inc)
opalt23(0xdf, getc, ramb, romb)
op15   (0xe0, dec)
opa123 (0xef, getb, getbh, getbl, getbs)
op16a12(0xf0, iwt, lm, sm)

#undef op
#undef op4
#undef op12
#undef op15
#undef op16
#undef opalt1
#undef op6alt1
#undef op15a1
#undef op16a1
#undef op16a3
#undef op16a12
#undef opalt23
#undef opa123
#undef opb
#undef bge
#undef blt
#undef bne
#undef beq
#undef bpl
#undef bmi
#undef bcc
#undef bcs
#undef bvc
#undef bvs

  }
}
#endif
