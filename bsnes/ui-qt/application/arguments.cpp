
void Application::parseArguments() {
  const QStringList argv = app->arguments();

  string fileName  = "";
  string slotAName = "";
  string slotBName = "";
  bool processSwitches = true;
  loadType = SNES::Cartridge::Mode::Normal;

  for(unsigned i = 1; i < argv.size(); i++) {
    const string arg = argv.at(i).toUtf8().data();

    if(arg == "--") { processSwitches = false; continue; }

    if(arg[0] == '-' && processSwitches) {
      const string param = i + 1 < argv.size() ? argv.at(i + 1).toUtf8().data() : "";

      bool usesParam = parseArgumentSwitch(arg, param);
      if (usesParam) { i++; continue; }
    } else {
      if     (fileName == "")  fileName = arg;
      else if(slotAName == "") slotAName = arg;
      else if(slotBName == "") slotBName = arg;
    }
  }

  if(fileName != "") {
    switch(loadType) {
    default: 
      loadCartridge(fileName); 
      break;
    case SNES::Cartridge::Mode::BsxSlotted:
      cartridge.loadBsxSlotted(fileName, slotAName);
      break;
    case SNES::Cartridge::Mode::Bsx:
      cartridge.loadBsx(fileName, slotAName);
      break;
    case SNES::Cartridge::Mode::SufamiTurbo:
      cartridge.loadSufamiTurbo(fileName, slotAName, slotBName);
      break;
    case SNES::Cartridge::Mode::SuperGameBoy:
      cartridge.loadSuperGameBoy(fileName, slotAName);
      break;
    }
  }
}

// returns true if argument uses parameter
bool Application::parseArgumentSwitch(const string& arg, const string& parameter) {
  if(arg == "--help" || arg == "-h") 
    { printArguments(); return false; }
  
  if(arg == "--bs-x" || arg == "-bs") 
    { loadType = SNES::Cartridge::Mode::Bsx; return false; }
  if(arg == "--bs-x-slotted" || arg == "-bss") 
    { loadType = SNES::Cartridge::Mode::BsxSlotted; return false; }
  if(arg == "--sufami-turbo" || arg == "-st") 
    { loadType = SNES::Cartridge::Mode::SufamiTurbo; return false; }
  if(arg == "--super-game-boy" || arg == "-sgb") 
    { loadType = SNES::Cartridge::Mode::SuperGameBoy; return false; }

  #if defined(DEBUGGER)
  if(arg == "--show-debugger") { debugger->show(); return false; }

  if(arg == "--break-immediately") {
    application.debug = true;
    application.debugrun = false;
    debugger->synchronize();
    return false;
  }

  if(arg == "--break-on-brk") {
    breakpointEditor->setBreakOnBrk(true);
    return false;
  }

  if(arg == "--breakpoint" || arg == "-b") {
    if(parameter == "" || parameter[0] == '-') return false;

    breakpointEditor->addBreakpoint(parameter);

    return true;
  }
  #endif

  return false;
}

void Application::printArguments() {
  string filepath = app->applicationFilePath().toUtf8().data();

  puts(string("Usage: ", notdir(filepath), " [options] filename(s)\n"));
  puts("  -h / --help                       show help\n"
       "  -bs / --bs-x                      load BS-X cartridge\n"
       "  -bss / --bs-x-slotted             load BS-X slotted cartridge\n"
       "  -st / --sufami-turbo              load Sufami Turbo cartridge\n"
       "  -sgb / --super-game-boy           load Super Game Boy cartridge\n"
       "\n"
       "For the above special cartridge types, specify the base cartridge "
       "followed by the (optional) slot cartridge(s).");
  #if defined(DEBUGGER)
  puts("\n"
       "  --show-debugger                   open debugger window on startup\n"
       "  --break-immediately               breaks when loading the cartridge\n"
       "  --break-on-brk                    break on BRK opcodes\n"
       "  -b / --breakpoint <breakpoint>    add breakpoint\n"
       "\n"
       "Breakpoint format: <addr>[-<addr end>][=<value>][:<rwx>[:<source>]]\n"
       "                   rwx = read / write / execute flags\n"
       "                   source = cpu, smp, vram, oam, cgram, sa1, sfx, sgb");
  #endif
}
