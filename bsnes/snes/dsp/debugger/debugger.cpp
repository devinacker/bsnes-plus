#ifdef DSP_CPP

bool DSPDebugger::property(unsigned id, string &name, string &value) {
  unsigned n = 0;

  #define item(name_, value_) \
  if(id == n++) { \
    name = name_; \
    value = value_; \
    return true; \
  }

  item("Main Volume - Left", (unsigned)read(0x0c));
  item("Main Volume - Right", (unsigned)read(0x1c));
  item("Echo Volume - Left", (unsigned)read(0x2c));
  item("Echo Volume - Right", (unsigned)read(0x3c));
  item("Key On", string("0x", hex<2>(read(0x4c))));
  item("Key Off", string("0x", hex<2>(read(0x5c))));
  item("Flag - Reset", (bool)(read(0x6c) & 0x80));
  item("Flag - Mute", (bool)(read(0x6c) & 0x40));
  item("Flag - Echo Disable", (bool)(read(0x6c) & 0x20));
  item("Flag - Noise Clock", (unsigned)read(0x6c) & 0x1f);
  item("Voice End Flags", string("0x", hex<2>(read(0x7c))));
  item("Pitch Modulation Enable", string("0x", hex<2>(read(0x2d))));
  item("Noise Enable", string("0x", hex<2>(read(0x3d))));
  item("Echo Enable", string("0x", hex<2>(read(0x4d))));
  item("Source Directory", string("0x", hex<4>(read(0x5d) << 8)));
  item("Echo Start Address", string("0x", hex<4>(read(0x6d) << 8)));
  item("Echo Length", string("0x", hex<4>(read(0x7d) << 11)));
  item("Echo Feedback", (unsigned)read(0x0d));

  for(unsigned i = 0; i < 8; i++) {
    item(string("Coefficient ", i), string("0x", hex<2>(read((i << 4) + 0x0f))));
  }

  static const char* const gainModes[] = {
    "0 (Linear Decrease)",
    "1 (Exponential Decrease)",
    "2 (Linear Increase)",
    "3 (Bent Line Increase)"
  };

  for(unsigned i = 0; i < 8; i++) {
    item(string("Voice ", i), "");
    item("Volume - Left", (unsigned)read((i << 4) + 0x00));
    item("Volume - Right", (unsigned)read((i << 4) + 0x01));
    item("Pitch Height", string("0x", hex<4>(read((i << 4) + 0x02) + (read((i << 4) + 0x03) << 8))));
    item("Source Number", (unsigned)read((i << 4) + 0x04));

    item("ADSR/Gain Select", (read((i << 4) + 0x05) & 0x80) ? "ADSR" : "Gain");
    item("Attack Rate", (unsigned)read((i << 4) + 0x05) & 0xf);
    item("Decay Rate", (unsigned)(read((i << 4) + 0x05) & 0x70) >> 4);
    item("Sustain Rate", (unsigned)read((i << 4) + 0x06) & 0x1f);
    item("Sustain Level", (unsigned)(read((i << 4) + 0x05) & 0xe0) >> 5);

    item("Gain Mode", (read((i << 4) + 0x07) & 0x80)
                      ? gainModes[(read((i << 4) + 0x07) & 0x60) >> 5]
                      : "Fixed Volume");
    item("Gain Level/Rate", (read((i << 4) + 0x07) & 0x80)
                            ? ((unsigned)read((i << 4) + 0x07) & 0x1f)
                            : ((unsigned)read((i << 4) + 0x07) & 0x7f));

    item("Envelope Output", (unsigned)read((i << 4) + 0x08));
    item("Sample Output", (unsigned)read((i << 4) + 0x09));
  }

  #undef item
  return false;
}

#endif
