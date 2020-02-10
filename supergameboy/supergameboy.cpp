#include "supergameboy.hpp"

#ifdef _WIN32
  #define bsnesexport __declspec(dllexport)
#else
  #define bsnesexport
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <interface/interface.cpp>

bsnesexport void sgb_rom(uint8_t *data, unsigned size) {
  supergameboy.romdata = data;
  supergameboy.romsize = size;
}

bsnesexport void sgb_ram(uint8_t *data, unsigned size) {
  supergameboy.ramdata = data;
  supergameboy.ramsize = size;
}

bsnesexport void sgb_rtc(uint8_t *data, unsigned size) {
  supergameboy.rtcdata = data;
  supergameboy.rtcsize = size;
}

bsnesexport bool sgb_init(bool version) {
  return supergameboy.init(version);
}

bsnesexport void sgb_term() {
  supergameboy.term();
}

bsnesexport void sgb_power() {
  supergameboy.power();
}

bsnesexport void sgb_reset() {
  supergameboy.reset();
}

bsnesexport uint8_t sgb_read(uint16_t addr) {
  return supergameboy.read(addr);
}

bsnesexport void sgb_write(uint16_t addr, uint8_t data) {
  supergameboy.write(addr, data);
}

bsnesexport unsigned sgb_run(uint32_t *samplebuffer, unsigned clocks) {
  return supergameboy.run(samplebuffer, clocks);
}

bsnesexport void sgb_save() {
  supergameboy.save();
}

bsnesexport void sgb_serialize(nall::serializer &s) {
  supergameboy.serialize(s);
}

bsnesexport uint8_t sgb_read_gb(uint16_t addr) {
  return supergameboy.read_gb(addr);
}

bsnesexport void sgb_write_gb(uint16_t addr, uint8_t data) {
  if (supergameboy.gambatte_)
    supergameboy.write_gb(addr, data);
}

bsnesexport uint16_t sgb_get_reg(char reg) {
  if (!supergameboy.gambatte_) return 0;

  return supergameboy.gambatte_->debugGetRegister(reg);
}

bsnesexport void sgb_set_reg(char reg, uint16_t value) {
  if (supergameboy.gambatte_)
    supergameboy.gambatte_->debugSetRegister(reg, value);
}

bsnesexport bool sgb_get_flag(char flag) {
  if (!supergameboy.gambatte_) return false;

  return supergameboy.gambatte_->debugGetFlag(flag);
}

bsnesexport void sgb_set_flag(char flag, bool value) {
  supergameboy.gambatte_->debugSetFlag(flag, value);
}

bsnesexport void sgb_callback_step(void (*step)(uint16_t)) {
  supergameboy.op_step = step;
}

bsnesexport void sgb_callback_call(void (*call)(uint16_t)) {
  supergameboy.op_call = call;
}

bsnesexport void sgb_callback_ret(void (*ret)(uint16_t)) {
  supergameboy.op_ret = ret;
}

bsnesexport void sgb_callback_irq(void (*irq)(uint16_t)) {
  supergameboy.op_irq = irq;
}

bsnesexport void sgb_callback_read(void (*read)(uint16_t, uint8_t)) {
  supergameboy.op_read = read;
}

bsnesexport void sgb_callback_readpc(void (*readpc)(uint16_t, uint8_t)) {
  supergameboy.op_readpc = readpc;
}

bsnesexport void sgb_callback_write(void (*write)(uint16_t, uint8_t)) {
  supergameboy.op_write = write;
}

