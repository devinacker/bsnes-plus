#include "registers.hpp"

uint8 color(uint8 source);
void plot(uint8 x, uint8 y);
uint8 rpix(uint8 x, uint8 y);
void pixelcache_flush(pixelcache_t &cache);

//opcode_table.cpp
inline uint8 op_exec(uint8 opcode);

//opcodes.cpp
unsigned reg_or_imm(unsigned n);

uint8 op_stop();
uint8 op_nop();
uint8 op_cache();
uint8 op_lsr();
uint8 op_rol();
uint8 op_bra();
uint8 op_nobranch();
uint8 op_to(unsigned n);
uint8 op_move(unsigned n);
uint8 op_with(unsigned n);
uint8 op_stw_stb(unsigned n);
uint8 op_loop();
uint8 op_alt1();
uint8 op_alt2();
uint8 op_alt3();
uint8 op_ldw_ldb(unsigned n);
uint8 op_plot();
uint8 op_rpix();
uint8 op_swap();
uint8 op_color();
uint8 op_cmode();
uint8 op_not();
uint8 op_add_adc(unsigned n);
uint8 op_sub_sbc(unsigned n);
uint8 op_cmp(unsigned n);
uint8 op_merge();
uint8 op_and_bic(unsigned n);
uint8 op_mult(unsigned n);
uint8 op_umult(unsigned n);
uint8 op_sbk();
uint8 op_link(unsigned n);
uint8 op_sex();
uint8 op_asr_div2();
uint8 op_ror();
uint8 op_jmp(unsigned n);
uint8 op_ljmp(unsigned n);
uint8 op_lob();
uint8 op_fmult_lmult();
uint8 op_ibt(unsigned n);
uint8 op_lms(unsigned n);
uint8 op_sms(unsigned n);
uint8 op_from(unsigned n);
uint8 op_moves(unsigned n);
uint8 op_hib();
uint8 op_or(unsigned n);
uint8 op_xor(unsigned n);
uint8 op_inc(unsigned n);
uint8 op_getc();
uint8 op_ramb();
uint8 op_romb();
uint8 op_dec(unsigned n);
uint8 op_getb();
uint8 op_getbh();
uint8 op_getbl();
uint8 op_getbs();
uint8 op_iwt(unsigned n);
uint8 op_lm(unsigned n);
uint8 op_sm(unsigned n);
