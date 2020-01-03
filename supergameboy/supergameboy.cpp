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

bsnesexport void sgb_row(unsigned row) {
  supergameboy.row(row);
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
  supergameboy.write_gb(addr, data);
}
