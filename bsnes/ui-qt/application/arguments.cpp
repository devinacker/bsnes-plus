
void Application::parseArguments() {
  const QStringList argv = app->arguments();

  string fileName = "";
  bool processSwitches = true;

  for(unsigned i = 1; i < argv.size(); i++) {
    const string arg = argv.at(i).toUtf8().data();

    if(arg == "--") { processSwitches = false; continue; }

    if(arg[0] == '-' && processSwitches) {
      const string param = i + 1 < argv.size() ? argv.at(i + 1).toUtf8().data() : "";

      bool usesParam = parseArgumentSwitch(arg, param);
      if (usesParam) { i++; continue; }
    } else {
      if(fileName == "") fileName = arg;
    }
  }

  if(fileName != "") {
    cartridge.loadNormal(fileName);
  }
}

// returns true if argument uses parameter
bool Application::parseArgumentSwitch(const string& arg, const string& parameter) {
  if(arg == "--help" || arg == "-h") { printArguments(); return false; }

  #if defined(DEBUGGER)
  if(arg == "--show-debugger") { debugger->show(); return false; }

  if(arg == "--breakpoint" || arg == "-b") {
    if(parameter == "" || parameter[0] == '-') return false;

    lstring param;
    param.split<3>(":", string(parameter).lower());
    if(param.size() == 1) { param.append("rwx"); }
    if(param.size() == 2) { param.append("cpu"); }

    breakpointEditor->addBreakpoint(param[0], param[1], param[2]);

    return true;
  }
  #endif

  return false;
}

void Application::printArguments() {
  string filepath = app->applicationFilePath().toUtf8().data();

  puts(string("Usage: ", notdir(filepath), " [options] filename\n"));
  puts("  -h / --help                       show help");
  #if defined(DEBUGGER)
  puts("  --show-debugger                   open debugger window on startup\n"
       "  -b / --breakpoint <breakpoint>    add breakpoint\n"
       "\n"
       "Breakpoint format: <addr>[-<addr end>][:<rwx>[:<source>]]\n"
       "                   rwx = read / write / execute flags\n"
       "                   source = cpu, smp, vram, oam, cgram, sa1, sfx");
  #endif
}
