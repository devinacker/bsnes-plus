#include <snes.hpp>

#define SUPERGAMEBOY_CPP
namespace SNES {

#if defined(DEBUGGER)
  #include "debugger/debugger.cpp"
  SGBDebugger supergameboy;
#else
  SuperGameBoy supergameboy;
#endif

#include "serialization.cpp"

void SuperGameBoy::Enter() { supergameboy.enter(); }

void SuperGameBoy::enter() {
  if(!sgb_run) while(true) {
    scheduler.synchronize();

    sample(0, 0);
    step(1);
    synchronize_cpu();
  }

  while(true) {
    scheduler.synchronize();

    unsigned samples = sgb_run(samplebuffer, 16);
    for(unsigned i = 0; i < samples; i++) {
      int16 left  = samplebuffer[i] >>  0;
      int16 right = samplebuffer[i] >> 16;

      //SNES audio is notoriously quiet; lower Game Boy samples to match SGB sound effects
      sample(left / 3, right / 3);
    }

    step(samples * speed * 2);
    synchronize_cpu();
  }
}

void SuperGameBoy::save() {
  if(sgb_save) sgb_save();
}

uint8 SuperGameBoy::read(unsigned addr) {
  if(sgb_read && !Memory::debugger_access()) {
    cpu.synchronize_coprocessor();
    return sgb_read(addr);
  }
  return 0x00;
}

void SuperGameBoy::write(unsigned addr, uint8 data) {
  if (!Memory::debugger_access()) {
    cpu.synchronize_coprocessor();
    if(sgb_write) sgb_write(addr, data);
    if(addr == 0x6003) {
      switch(data & 3) {
      case 0: speed = 4; break;
      case 1: speed = 5; break;
      case 2: speed = 7; break;
      case 3: speed = 9; break;
      }
      update_speed();
    }
  }
}

void SuperGameBoy::init() {
  if(open("supergameboy")) {
    sgb_rom       = sym("sgb_rom");
    sgb_ram       = sym("sgb_ram");
    sgb_rtc       = sym("sgb_rtc");
    sgb_init      = sym("sgb_init");
    sgb_term      = sym("sgb_term");
    sgb_power     = sym("sgb_power");
    sgb_reset     = sym("sgb_reset");
    sgb_read      = sym("sgb_read");
    sgb_write     = sym("sgb_write");
    sgb_run       = sym("sgb_run");
    sgb_save      = sym("sgb_save");
    sgb_serialize = sym("sgb_serialize");
  }
}

void SuperGameBoy::enable() {
}

void SuperGameBoy::power() {
  unsigned frequency_ = (cartridge.supergameboy_version() == Cartridge::SuperGameBoyVersion::Version1 ? system.cpu_frequency() : 20971520);
  create(SuperGameBoy::Enter, frequency_);

  audio.add_stream(this);

  if(sgb_rom) sgb_rom(memory::gbrom.data(), memory::gbrom.size());
  if(sgb_ram) sgb_ram(memory::gbram.data(), memory::gbram.size());
  if(sgb_rtc) sgb_rtc(memory::gbrtc.data(), memory::gbrtc.size());

  bool version = (cartridge.supergameboy_version() == Cartridge::SuperGameBoyVersion::Version1) ? 0 : 1;
  if(sgb_init) sgb_init(version);
  if(sgb_power) sgb_power();
  
  speed = 1;
  update_speed();
}

void SuperGameBoy::reset() {
  create(SuperGameBoy::Enter, frequency);

  if(sgb_reset) sgb_reset();
}

void SuperGameBoy::unload() {
  if(sgb_term) sgb_term();
}

void SuperGameBoy::update_speed() {
  audio_frequency((double)frequency / (speed * 2));
}

}
