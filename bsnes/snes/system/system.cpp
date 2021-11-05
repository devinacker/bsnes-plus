#include <snes.hpp>

#define SYSTEM_CPP
namespace SNES {

System system;

#include <config/config.cpp>
#include <debugger/debugger.cpp>
#include <scheduler/scheduler.cpp>

#include <video/video.cpp>
#include <audio/audio.cpp>
#include <input/input.cpp>

#include "random.cpp"
#include "serialization.cpp"

void System::run() {
  scheduler.mode = Scheduler::Mode::Run;

  scheduler.enter();
  if(scheduler.exit_reason() == Scheduler::ExitReason::FrameEvent) {
    input.update();
    video.update();
  }
}

void System::runtosave() {
  scheduler.mode = Scheduler::Mode::Synchronize;
 
  //run every thread until it cleanly hits a synchronization point
  //if it fails, start resynchronizing every thread again
  while (true) {
    if(SMP::Threaded == true) {
      if (!runthreadtosave(smp.thread)) continue;
    }
    if(CPU::Threaded == true) {
      if (!runthreadtosave(cpu.thread)) continue;
    }
    if(PPU::Threaded == true) {
      if (!runthreadtosave(ppu.thread)) continue;
    }
    if(DSP::Threaded == true) {
      if (!runthreadtosave(dsp.thread)) continue;
    }
	bool synchronized = true;
    for(unsigned i = 0; i < cpu.coprocessors.size(); i++) {
      Processor &chip = *cpu.coprocessors[i];
      synchronized &= runthreadtosave(chip.thread);
    }
	if (synchronized) break;
  }
  
  scheduler.mode = Scheduler::Mode::Run;
  scheduler.thread = cpu.thread;
}

bool System::runthreadtosave(cothread_t& thread) {
  scheduler.thread = thread;
  while(true) {
    scheduler.enter();
    if(scheduler.exit_reason() == Scheduler::ExitReason::SynchronizeEvent) break;
    if(scheduler.exit_reason() == Scheduler::ExitReason::DesynchronizeEvent) return false;
    if(scheduler.exit_reason() == Scheduler::ExitReason::FrameEvent) {
      input.update();
      video.update();
    }
  }
  return true;
}

void System::init(Interface *interface_) {
  intf = interface_;
  assert(intf != 0);

  supergameboy.init();
  superfx.init();
  sa1.init();
  necdsp.init();
  bsxbase.init();
  bsxcart.init();
  bsxflash.init();
  srtc.init();
  sdd1.init();
  spc7110.init();
  cx4.init();
  obc1.init();
  st0018.init();
  msu1.init();
  serial.init();

  video.init();
  audio.init();
  input.init();

  input.port_set_device(0, config().controller_port1);
  input.port_set_device(1, config().controller_port2);
}

void System::term() {
}

void System::power() {
  random.seed((unsigned)time(0));

  region = config().region;
  expansion = config().expansion_port;
  if(region == Region::Autodetect) {
    region = (cartridge.region() == Cartridge::Region::NTSC ? Region::NTSC : Region::PAL);
  }

  cpu_frequency = region() == Region::NTSC ? config().cpu.ntsc_frequency : config().cpu.pal_frequency;
  apu_frequency = region() == Region::NTSC ? config().smp.ntsc_frequency : config().smp.pal_frequency;

  bus.power();
  memory::mmio.map(0x2100, 0x213f, ppu);
  memory::mmio.map(0x2140, 0x217f, smp);
  memory::mmio.map(0x2180, 0x2183, cpu);
  memory::mmio.map(0x4016, 0x4017, cpu);
  memory::mmio.map(0x4200, 0x421f, cpu);
  memory::mmio.map(0x4300, 0x437f, cpu);

  audio.init();
  if(expansion == ExpansionPortDevice::BSX) bsxbase.enable();
  if(cartridge.bsxpack_type() == Cartridge::BSXPackType::FlashROM) bsxflash.enable();
  if(cartridge.mode() == Cartridge::Mode::Bsx) bsxcart.enable();
  if(cartridge.mode() == Cartridge::Mode::SuperGameBoy) supergameboy.enable();

  if(cartridge.has_superfx()) superfx.enable();
  if(cartridge.has_sa1()) sa1.enable();
  if(cartridge.has_necdsp()) necdsp.enable();
  if(cartridge.has_srtc()) srtc.enable();
  if(cartridge.has_sdd1()) sdd1.enable();
  if(cartridge.has_spc7110()) spc7110.enable();
  if(cartridge.has_cx4()) cx4.enable();
  if(cartridge.has_obc1()) obc1.enable();
  if(cartridge.has_st0018()) st0018.enable();
  if(cartridge.has_msu1()) msu1.enable();
  if(cartridge.has_serial()) serial.enable();

  cpu.power();
  smp.power();
  dsp.power();
  ppu.power();

  // always initialize expansion port device(s) so they are handled correctly in savestates
  bsxbase.power();
  
  if(cartridge.bsxpack_type() == Cartridge::BSXPackType::FlashROM) bsxflash.power();
  if(cartridge.mode() == Cartridge::Mode::Bsx) bsxcart.power();
  if(cartridge.mode() == Cartridge::Mode::SuperGameBoy) supergameboy.power();

  if(cartridge.has_superfx()) superfx.power();
  if(cartridge.has_sa1()) sa1.power();
  if(cartridge.has_necdsp()) necdsp.power();
  if(cartridge.has_srtc()) srtc.power();
  if(cartridge.has_sdd1()) sdd1.power();
  if(cartridge.has_spc7110()) spc7110.power();
  if(cartridge.has_cx4()) cx4.power();
  if(cartridge.has_obc1()) obc1.power();
  if(cartridge.has_st0018()) st0018.power();
  if(cartridge.has_msu1()) msu1.power();
  if(cartridge.has_serial()) serial.power();

  if(expansion == ExpansionPortDevice::BSX) cpu.coprocessors.append(&bsxbase);
  if(cartridge.mode() == Cartridge::Mode::SuperGameBoy) cpu.coprocessors.append(&supergameboy);
  if(cartridge.has_superfx()) cpu.coprocessors.append(&superfx);
  if(cartridge.has_sa1()) cpu.coprocessors.append(&sa1);
  if(cartridge.has_necdsp()) cpu.coprocessors.append(&necdsp);
  if(cartridge.has_cx4()) cpu.coprocessors.append(&cx4);
  if(cartridge.has_msu1()) cpu.coprocessors.append(&msu1);
  if(cartridge.has_serial()) cpu.coprocessors.append(&serial);

  scheduler.init();

  input.update();
//video.update();
}

void System::reset() {
  bus.reset();
  cpu.reset();
  smp.reset();
  dsp.reset();
  ppu.reset();

  // always initialize expansion port device(s) so they are handled correctly in savestates
  bsxbase.reset();
  
  if(cartridge.bsxpack_type() == Cartridge::BSXPackType::FlashROM) bsxflash.reset();
  if(cartridge.mode() == Cartridge::Mode::Bsx) bsxcart.reset();
  if(cartridge.mode() == Cartridge::Mode::SuperGameBoy) supergameboy.reset();

  if(cartridge.has_superfx()) superfx.reset();
  if(cartridge.has_sa1()) sa1.reset();
  if(cartridge.has_necdsp()) necdsp.reset();
  if(cartridge.has_srtc()) srtc.reset();
  if(cartridge.has_sdd1()) sdd1.reset();
  if(cartridge.has_spc7110()) spc7110.reset();
  if(cartridge.has_cx4()) cx4.reset();
  if(cartridge.has_obc1()) obc1.reset();
  if(cartridge.has_st0018()) st0018.reset();
  if(cartridge.has_msu1()) msu1.reset();
  if(cartridge.has_serial()) serial.reset();

  if(expansion == ExpansionPortDevice::BSX) cpu.coprocessors.append(&bsxbase);
  if(cartridge.mode() == Cartridge::Mode::SuperGameBoy) cpu.coprocessors.append(&supergameboy);
  if(cartridge.has_superfx()) cpu.coprocessors.append(&superfx);
  if(cartridge.has_sa1()) cpu.coprocessors.append(&sa1);
  if(cartridge.has_necdsp()) cpu.coprocessors.append(&necdsp);
  if(cartridge.has_cx4()) cpu.coprocessors.append(&cx4);
  if(cartridge.has_msu1()) cpu.coprocessors.append(&msu1);
  if(cartridge.has_serial()) cpu.coprocessors.append(&serial);

  scheduler.init();

  input.port_set_device(0, config().controller_port1);
  input.port_set_device(1, config().controller_port2);
  input.update();
//video.update();
}

void System::unload() {
  bsxbase.unload();
  if(cartridge.mode() == Cartridge::Mode::SuperGameBoy) supergameboy.unload();
  
  if(cartridge.has_msu1()) msu1.unload();
}

void System::scanline() {
  video.scanline();
  if(cpu.vcounter() == 241) scheduler.exit(Scheduler::ExitReason::FrameEvent);
}

void System::frame() {
}

System::System() : intf(0) {
  region = Region::Autodetect;
  expansion = ExpansionPortDevice::None;
}

}
