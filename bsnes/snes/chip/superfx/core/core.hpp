#include "registers.hpp"

uint8 color(uint8 source);
void plot(uint8 x, uint8 y);
uint8 rpix(uint8 x, uint8 y);
void pixelcache_flush(pixelcache_t &cache);

//opcode_table.cpp
inline void op_exec(uint8 opcode);

//opcodes.cpp
unsigned reg_or_imm(unsigned n);

void op_stop();
void op_nop();
void op_cache();
void op_lsr();
void op_rol();
void op_bra();
void op_bge();
void op_blt();
void op_bne();
void op_beq();
void op_bpl();
void op_bmi();
void op_bcc();
void op_bcs();
void op_bvc();
void op_bvs();
void op_to_move(unsigned n);
void op_with(unsigned n);
void op_stw_stb(unsigned n);
void op_loop();
void op_alt1();
void op_alt2();
void op_alt3();
void op_ldw_ldb(unsigned n);
void op_plot();
void op_rpix();
void op_swap();
void op_color();
void op_cmode();
void op_not();
void op_add_adc(unsigned n);
void op_sub_sbc(unsigned n);
void op_cmp(unsigned n);
void op_merge();
void op_and_bic(unsigned n);
void op_mult(unsigned n);
void op_umult(unsigned n);
void op_sbk();
void op_link(unsigned n);
void op_sex();
void op_asr_div2();
void op_ror();
void op_jmp(unsigned n);
void op_ljmp(unsigned n);
void op_lob();
void op_fmult_lmult();
void op_ibt(unsigned n);
void op_lms(unsigned n);
void op_sms(unsigned n);
void op_from_moves(unsigned n);
void op_hib();
void op_or(unsigned n);
void op_xor(unsigned n);
void op_inc(unsigned n);
void op_getc();
void op_ramb();
void op_romb();
void op_dec(unsigned n);
void op_getb();
void op_getbh();
void op_getbl();
void op_getbs();
void op_iwt(unsigned n);
void op_lm(unsigned n);
void op_sm(unsigned n);
