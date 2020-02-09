#ifndef SUPERGAMEBOY_HPP
#define SUPERGAMEBOY_HPP

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <nall/file.hpp>
#include <nall/serializer.hpp>
using namespace nall;

#include <libgambatte/include/gambatte.h>
#include <interface/interface.hpp>

extern "C" {
  void sgb_rom(uint8_t *data, unsigned size);
  void sgb_ram(uint8_t *data, unsigned size);
  void sgb_rtc(uint8_t *data, unsigned size);
  bool sgb_init(bool version);
  void sgb_term();
  void sgb_power();
  void sgb_reset();
  uint8_t sgb_read(uint16_t addr);
  void sgb_write(uint16_t addr, uint8_t data);
  unsigned sgb_run(uint32_t *samplebuffer, unsigned clocks);
  void sgb_save();
  void sgb_serialize(nall::serializer &s);
  
  // debugger
  uint8_t sgb_read_gb(uint16_t addr);
  void    sgb_write_gb(uint16_t addr, uint8_t data);
  
  uint16_t sgb_get_reg(char reg);
  void     sgb_set_reg(char reg, uint16_t value);
  bool     sgb_get_flag(char flag);
  void     sgb_set_flag(char flag, bool value);
  
  void sgb_callback_step(void (*step)(uint16_t));
  void sgb_callback_call(void (*step)(uint16_t));
  void sgb_callback_ret(void (*step)(uint16_t));
  void sgb_callback_irq(void (*step)(uint16_t));
  void sgb_callback_read(void (*read)(uint16_t, uint8_t));
  void sgb_callback_readpc(void (*readpc)(uint16_t, uint8_t));
  void sgb_callback_write(void (*write)(uint16_t, uint8_t));
}

#endif
