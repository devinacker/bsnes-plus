#ifdef SYSTEM_CPP

Debugger debugger;

void Debugger::breakpoint_test(Debugger::Breakpoint::Source source, Debugger::Breakpoint::Mode mode, unsigned addr, uint8 data) {
  for(unsigned i = 0; i < Breakpoints; i++) {
    if(breakpoint[i].enabled == false) continue;

    if(breakpoint[i].data != -1 && breakpoint[i].data != data) continue;
    if(breakpoint[i].source != source) continue;
    if(breakpoint[i].mode != mode) continue;

    // account for address mirroring on the S-CPU and SA-1 buses
    if (source == Debugger::Breakpoint::Source::CPUBus) {
      Bus::Page &source_page = bus.page[breakpoint[i].addr >> 8];
      Bus::Page &offset_page = bus.page[addr >> 8];

      if (source_page.access != offset_page.access) continue;
      if (source_page.offset + breakpoint[i].addr != offset_page.offset + addr) continue;
    } else if (source == Debugger::Breakpoint::Source::SA1Bus) {
      Bus::Page &source_page = sa1bus.page[breakpoint[i].addr >> 8];
      Bus::Page &offset_page = sa1bus.page[addr >> 8];

      if (source_page.access != offset_page.access) continue;
      if (source_page.offset + breakpoint[i].addr != offset_page.offset + addr) continue;
    } else {
      if (breakpoint[i].addr != addr) continue;
    }
    
    breakpoint[i].counter++;
    breakpoint_hit = i;
    break_event = BreakEvent::BreakpointHit;
    scheduler.exit(Scheduler::ExitReason::DebuggerEvent);
    break;
  }
}

uint8 Debugger::read(Debugger::MemorySource source, unsigned addr) {
  switch(source) {
    case MemorySource::CPUBus: {
      //do not read from memory-mapped registers that could affect program behavior
      if(((addr - 0x2000) & 0x40c000) == 0x000000) break;  //$00-3f:2000-5fff MMIO
      return bus.read(addr & 0xffffff);
    } break;

    case MemorySource::APUBus: {
      if((addr & 0xffc0) == 0xffc0) return smp.iplrom[addr & 0x3f];
      return memory::apuram.read(addr & 0xffff);
    } break;

    case MemorySource::APURAM: {
      return memory::apuram.read(addr & 0xffff);
    } break;

    case MemorySource::VRAM: {
      return memory::vram.read(addr & 0xffff);
    } break;

    case MemorySource::OAM: {
      if(addr & 0x0200) return memory::oam.read(0x0200 + (addr & 0x1f));
      return memory::oam.read(addr & 0x01ff);
    } break;

    case MemorySource::CGRAM: {
      return memory::cgram.read(addr & 0x01ff);
    } break;
    
    case MemorySource::CartROM: {
      if (addr < memory::cartrom.size())
        return memory::cartrom.read(addr & 0xffffff);
    } break;
    
    case MemorySource::CartRAM: {
      if (addr < memory::cartram.size())
        return memory::cartram.read(addr & 0xffffff);
    } break;
    
    case MemorySource::SA1Bus: {
      // VBR bus already excludes MMIO (and doesn't sync to the S-CPU like the normal SA-1 bus does)
      return cartridge.has_sa1() ? vbrbus.read(addr & 0xffffff) : 0;
    } break;
  }

  return 0x00;
}

void Debugger::write(Debugger::MemorySource source, unsigned addr, uint8 data) {
  switch(source) {
    case MemorySource::CPUBus: {
      //do not write to memory-mapped registers that could affect program behavior
      if(((addr - 0x2000) & 0x40c000) == 0x000000) break;  //$00-3f:2000-5fff MMIO
      memory::cartrom.write_protect(false);
      bus.write(addr & 0xffffff, data);
      memory::cartrom.write_protect(true);
    } break;

    case MemorySource::APURAM: {
      memory::apuram.write(addr & 0xffff, data);
    } break;

    case MemorySource::VRAM: {
      memory::vram.write(addr & 0xffff, data);
    } break;

    case MemorySource::OAM: {
      if(addr & 0x0200) memory::oam.write(0x0200 + (addr & 0x1f), data);
      else memory::oam.write(addr & 0x01ff, data);
    } break;

    case MemorySource::CGRAM: {
      memory::cgram.write(addr & 0x01ff, data);
    } break;
    
    case MemorySource::CartROM: {
      if (addr < memory::cartrom.size()) {
        memory::cartrom.write_protect(false);
        return memory::cartrom.write(addr & 0xffffff, data);
        memory::cartrom.write_protect(true);
      }
    } break;
    
    case MemorySource::CartRAM: {
      if (addr < memory::cartram.size())
        memory::cartram.write(addr & 0xffffff, data);
    } break;
    
    case MemorySource::SA1Bus: {
      // VBR bus already excludes MMIO (and doesn't sync to the S-CPU like the normal SA-1 bus does)
      if (cartridge.has_sa1()) vbrbus.write(addr & 0xffffff, data);
    } break;
  }
}

Debugger::Debugger() {
  break_event = BreakEvent::None;

  for(unsigned n = 0; n < Breakpoints; n++) {
    breakpoint[n].enabled = false;
    breakpoint[n].addr = 0;
    breakpoint[n].data = -1;
    breakpoint[n].mode = Breakpoint::Mode::Exec;
    breakpoint[n].source = Breakpoint::Source::CPUBus;
    breakpoint[n].counter = 0;
  }
  breakpoint_hit = 0;

  step_cpu = false;
  step_smp = false;
  step_sa1 = false;
  
  step_type = StepType::None;
}

#endif
