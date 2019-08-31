#ifdef SYSTEM_CPP

Configuration &config() {
  static Configuration configuration;
  return configuration;
}

Configuration::Configuration() {
  controller_port1 = Input::Device::Joypad;
  controller_port2 = Input::Device::Joypad;
  expansion_port   = System::ExpansionPortDevice::BSX;
  region           = System::Region::Autodetect;
  random           = true;

  cpu.version         = 2;
  cpu.ntsc_frequency  = 21477272;  //315 / 88 * 6000000
  cpu.pal_frequency   = 21281370;
  cpu.wram_init_value = 0x55;

  smp.ntsc_frequency = 24607104;   //32040.5 * 768
  smp.pal_frequency  = 24607104;

  ppu1.version = 1;
  ppu2.version = 3;

  path.bsxdat = "./bsxdat/";
  
  sat.local_time = true;
  sat.custom_time = 798653040; // 1995-04-23 16:04
  sat.default_size = 2; // 8 Mbit
}

#endif
