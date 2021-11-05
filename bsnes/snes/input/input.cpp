#ifdef SYSTEM_CPP

Input input;

uint8 Input::port_read(bool portnumber) {
  if(cartridge.has_serial() && portnumber == 1) {
    return (serial.data2() << 1) | (serial.data1() << 0);
  }

  port_t &p = port[portnumber];

  switch(p.device) {
    case Device::Joypad:
    case Device::SGBCommander: {
      if(cpu.joylatch() == 0) {
        if(p.counter0 >= 16) return 1;
        return system.intf->input_poll(portnumber, p.device, 0, p.counter0++);
      } else {
        return system.intf->input_poll(portnumber, p.device, 0, 0);
      }
    } //case Device::Joypad

    case Device::Multitap: {
      if(cpu.joylatch()) return 2; //when latch is high -- data2 = 1, data1 = 0

      unsigned deviceidx, deviceindex0, deviceindex1;
      uint8 mask = (portnumber == 0 ? 0x40 : 0x80);

      if(cpu.pio() & mask) {
        deviceidx = p.counter0;
        if(deviceidx >= 16) return 3;
        p.counter0++;

        deviceindex0 = 0;  //controller 1
        deviceindex1 = 1;  //controller 2
      } else {
        deviceidx = p.counter1;
        if(deviceidx >= 16) return 3;
        p.counter1++;

        deviceindex0 = 2;  //controller 3
        deviceindex1 = 3;  //controller 4
      }

      return (system.intf->input_poll(portnumber, p.device, deviceindex0, deviceidx) << 0)
           | (system.intf->input_poll(portnumber, p.device, deviceindex1, deviceidx) << 1);
    } //case Device::Multitap

    case Device::Mouse: {
      if(cpu.joylatch()) {
        p.mouse.speed++;
        if(p.mouse.speed > 2) p.mouse.speed = 0;
        return 0;
      }

      if(p.counter0 >= 32) return 1;

      switch(p.counter0++) { default:
        case  0: return 0;
        case  1: return 0;
        case  2: return 0;
        case  3: return 0;
        case  4: return 0;
        case  5: return 0;
        case  6: return 0;
        case  7: return 0;

        case  8: return system.intf->input_poll(portnumber, p.device, 0, (unsigned)MouseID::Right);
        case  9: return system.intf->input_poll(portnumber, p.device, 0, (unsigned)MouseID::Left);
        case 10: return (p.mouse.speed >> 1) & 1;  //speed (0 = slow, 1 = normal, 2 = fast, 3 = unused)
        case 11: return (p.mouse.speed >> 0) & 1;  // ||

        case 12: return 0;  //signature
        case 13: return 0;  // ||
        case 14: return 0;  // ||
        case 15: return 1;  // ||

        case 16: return p.mouse.dy;
        case 17: return (p.mouse.y >> 6) & 1;
        case 18: return (p.mouse.y >> 5) & 1;
        case 19: return (p.mouse.y >> 4) & 1;
        case 20: return (p.mouse.y >> 3) & 1;
        case 21: return (p.mouse.y >> 2) & 1;
        case 22: return (p.mouse.y >> 1) & 1;
        case 23: return (p.mouse.y >> 0) & 1;

        case 24: return p.mouse.dx;
        case 25: return (p.mouse.x >> 6) & 1;
        case 26: return (p.mouse.x >> 5) & 1;
        case 27: return (p.mouse.x >> 4) & 1;
        case 28: return (p.mouse.x >> 3) & 1;
        case 29: return (p.mouse.x >> 2) & 1;
        case 30: return (p.mouse.x >> 1) & 1;
        case 31: return (p.mouse.x >> 0) & 1;
      }
    } //case Device::Mouse

    case Device::SuperScope: {
      if(portnumber == 0) break;  //Super Scope in port 1 not supported ...
      if(p.counter0 >= 8) return 1;

      if(p.counter0 == 0) {
        //turbo is a switch; toggle is edge sensitive
        bool turbo = system.intf->input_poll(portnumber, p.device, 0, (unsigned)SuperScopeID::Turbo);
        if(turbo && !p.superscope.turbolock) {
          p.superscope.turbo = !p.superscope.turbo;  //toggle state
          p.superscope.turbolock = true;
        } else if(!turbo) {
          p.superscope.turbolock = false;
        }

        //trigger is a button
        //if turbo is active, trigger is level sensitive; otherwise it is edge sensitive
        p.superscope.trigger = false;
        bool trigger = system.intf->input_poll(portnumber, p.device, 0, (unsigned)SuperScopeID::Trigger);
        if(trigger && (p.superscope.turbo || !p.superscope.triggerlock)) {
          p.superscope.trigger = true;
          p.superscope.triggerlock = true;
        } else if(!trigger) {
          p.superscope.triggerlock = false;
        }

        //cursor is a button; it is always level sensitive
        p.superscope.cursor = system.intf->input_poll(portnumber, p.device, 0, (unsigned)SuperScopeID::Cursor);

        //pause is a button; it is always edge sensitive
        p.superscope.pause = false;
        bool pause = system.intf->input_poll(portnumber, p.device, 0, (unsigned)SuperScopeID::Pause);
        if(pause && !p.superscope.pauselock) {
          p.superscope.pause = true;
          p.superscope.pauselock = true;
        } else if(!pause) {
          p.superscope.pauselock = false;
        }

        p.superscope.offscreen =
           p.superscope.x < 0 || p.superscope.x >= 256
        || p.superscope.y < 0 || p.superscope.y >= (ppu.overscan() ? 240 : 225);
      }

      switch(p.counter0++) {
        case 0: return p.superscope.trigger;
        case 1: return p.superscope.cursor;
        case 2: return p.superscope.turbo;
        case 3: return p.superscope.pause;
        case 4: return 0;
        case 5: return 0;
        case 6: return p.superscope.offscreen;
        case 7: return 0;  //noise (1 = yes)
      }
    } //case Device::SuperScope

    case Device::Justifier:
    case Device::Justifiers: {
      if(portnumber == 0) break;  //Justifier in port 1 not supported ...
      if(p.counter0 >= 32) return 1;

      if(p.counter0 == 0) {
        p.justifier.trigger1 = system.intf->input_poll(portnumber, p.device, 0, (unsigned)JustifierID::Trigger);
        p.justifier.start1   = system.intf->input_poll(portnumber, p.device, 0, (unsigned)JustifierID::Start);

        if(p.device == Device::Justifiers) {
          p.justifier.trigger2 = system.intf->input_poll(portnumber, p.device, 1, (unsigned)JustifierID::Trigger);
          p.justifier.start2   = system.intf->input_poll(portnumber, p.device, 1, (unsigned)JustifierID::Start);
        } else {
          p.justifier.x2 = -1;
          p.justifier.y2 = -1;

          p.justifier.trigger2 = false;
          p.justifier.start2   = false;
        }
      }

      switch(p.counter0++) {
        case  0: return 0;
        case  1: return 0;
        case  2: return 0;
        case  3: return 0;
        case  4: return 0;
        case  5: return 0;
        case  6: return 0;
        case  7: return 0;
        case  8: return 0;
        case  9: return 0;
        case 10: return 0;
        case 11: return 0;

        case 12: return 1;  //signature
        case 13: return 1;  // ||
        case 14: return 1;  // ||
        case 15: return 0;  // ||

        case 16: return 0;
        case 17: return 1;
        case 18: return 0;
        case 19: return 1;
        case 20: return 0;
        case 21: return 1;
        case 22: return 0;
        case 23: return 1;

        case 24: return p.justifier.trigger1;
        case 25: return p.justifier.trigger2;
        case 26: return p.justifier.start1;
        case 27: return p.justifier.start2;
        case 28: return p.justifier.active;

        case 29: return 0;
        case 30: return 0;
        case 31: return 0;
      }
    } //case Device::Justifier(s)

    case Device::NTTDataKeypad: {
      if(cpu.joylatch() == 0) {
        if(p.counter0 >= 32) return 1;

		// Id bits for NTTDataKeypad
		if(p.counter0 >= 12 && p.counter0 <= 15) {
			if(p.counter0 == 13) {
				p.counter0++;
				return 1;
			}
			p.counter0++;
			return 0;
		}

        return system.intf->input_poll(portnumber, p.device, 0, p.counter0++);
      } else {
        return system.intf->input_poll(portnumber, p.device, 0, 0);
      }
    } //case Device::NTTDataKeypad


  } //switch(p.device)

  //no device connected
  return 0;
}

//scan all input; update cursor positions if needed
void Input::update() {
  system.intf->input_poll();
  port_t &p = port[1];

  switch(p.device) {
    case Device::SuperScope: {
      int x = system.intf->input_poll(1, p.device, 0, (unsigned)SuperScopeID::X);
      int y = system.intf->input_poll(1, p.device, 0, (unsigned)SuperScopeID::Y);
      x += p.superscope.x;
      y += p.superscope.y;
      p.superscope.x = max(-16, min(256 + 16, x));
      p.superscope.y = max(-16, min(240 + 16, y));

      latchx = p.superscope.x;
      latchy = p.superscope.y;
    } break;

    case Device::Justifier:
    case Device::Justifiers: {
      int x1 = system.intf->input_poll(1, p.device, 0, (unsigned)JustifierID::X);
      int y1 = system.intf->input_poll(1, p.device, 0, (unsigned)JustifierID::Y);
      x1 += p.justifier.x1;
      y1 += p.justifier.y1;
      p.justifier.x1 = max(-16, min(256 + 16, x1));
      p.justifier.y1 = max(-16, min(240 + 16, y1));

      int x2 = system.intf->input_poll(1, p.device, 1, (unsigned)JustifierID::X);
      int y2 = system.intf->input_poll(1, p.device, 1, (unsigned)JustifierID::Y);
      x2 += p.justifier.x2;
      y2 += p.justifier.y2;
      p.justifier.x2 = max(-16, min(256 + 16, x2));
      p.justifier.y2 = max(-16, min(240 + 16, y2));

      if(p.justifier.active == 0) {
        latchx = p.justifier.x1;
        latchy = p.justifier.y1;
      } else {
        latchx = (p.device == Device::Justifiers ? p.justifier.x2 : -1);
        latchy = (p.device == Device::Justifiers ? p.justifier.y2 : -1);
      }
    } break;
  }

  if(latchy < 0 || latchy >= (ppu.overscan() ? 240 : 225) || latchx < 0 || latchx >= 256) {
    //cursor is offscreen, set to invalid position so counters are not latched
    latchx = ~0;
    latchy = ~0;
  } else {
    //cursor is onscreen
    latchx += 40;  //offset trigger position to simulate hardware latching delay
    latchx <<= 2;  //dot -> clock conversion
    latchx +=  2;  //align trigger on half-dot ala interrupts (speed optimization for sCPU::add_clocks)
  }
}

void Input::port_set_device(bool portnumber, Device device) {
  port_t &p = port[portnumber];

  p.device = device;
  p.counter0 = 0;
  p.counter1 = 0;

  //set iobit to true if device is capable of latching PPU counters
  iobit = port[1].device == Device::SuperScope
       || port[1].device == Device::Justifier
       || port[1].device == Device::Justifiers;
  latchx = -1;
  latchy = -1;

  if(device == Device::Mouse) {
    p.mouse.dx = false;
    p.mouse.dy = false;

    p.mouse.x = 0;
    p.mouse.y = 0;

    p.mouse.speed = 0;
  } else if(device == Device::SuperScope) {
    p.superscope.x = 256 / 2;
    p.superscope.y = 240 / 2;

    p.superscope.trigger   = false;
    p.superscope.cursor    = false;
    p.superscope.turbo     = false;
    p.superscope.pause     = false;
    p.superscope.offscreen = false;

    p.superscope.turbolock   = false;
    p.superscope.triggerlock = false;
    p.superscope.pauselock   = false;
  } else if(device == Device::Justifier) {
    p.justifier.active = 0;
    p.justifier.x1 = 256 / 2;
    p.justifier.y1 = 240 / 2;
    p.justifier.x2 = -1;
    p.justifier.y2 = -1;

    p.justifier.trigger1 = false;
    p.justifier.trigger2 = false;
    p.justifier.start1 = false;
    p.justifier.start2 = false;
  } else if(device == Device::Justifiers) {
    p.justifier.active = 0;
    p.justifier.x1 = 256 / 2 - 16;
    p.justifier.y1 = 240 / 2;
    p.justifier.x2 = 256 / 2 + 16;
    p.justifier.y2 = 240 / 2;

    p.justifier.trigger1 = false;
    p.justifier.trigger2 = false;
    p.justifier.start1 = false;
    p.justifier.start2 = false;
  }
}

void Input::poll() {
  for(unsigned i = 0; i < 2; i++) {
    port[i].counter0 = 0;
    port[i].counter1 = 0;

    if(port[i].device == Device::Mouse) {
      int x = system.intf->input_poll(i, port[i].device, 0, (unsigned)MouseID::X);  //-n = left, 0 = center, +n = right
      int y = system.intf->input_poll(i, port[i].device, 0, (unsigned)MouseID::Y);  //-n = up,   0 = center, +n = down

      port[i].mouse.dx = x < 0;
      port[i].mouse.dy = y < 0;

      if(x < 0) x = -x;
      if(y < 0) y = -y;

      port[i].mouse.x = min(127, x);
      port[i].mouse.y = min(127, y);
    }
  }

  port[1].justifier.active = !port[1].justifier.active;
}

void Input::init() {
}

#endif
