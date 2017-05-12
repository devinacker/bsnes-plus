
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

    breakpointEditor->addBreakpoint(parameter);

    return true;
  }

  if(arg == "--break-on-wdm") { breakpointEditor->setBreakOnWDM(true); return false; }
  #endif

  return false;
}

void Application::printArguments() {
  string filepath = app->applicationFilePath().toUtf8().data();

  puts(string("Usage: ", notdir(filepath), " [options] filename\n"));
  puts("  -h / --help                       show help");
  #if defined(DEBUGGER)
  puts("  --show-debugger                   open debugger window on startup\n"
       "  --break-on-wdm                    break on wdm opcode\n"
       "  -b / --breakpoint <breakpoint>    add breakpoint\n"
       "\n"
<<<<<<< HEAD
       "  Breakpoint format: <addr>[-<addr end>][:<rwx>[:<source>]]\n"
       "                     rwx = read / write / execute flags\n"
       "                     source = cpu, smp, vram, oam, cgram, sa1, sfx"
       );
=======
       "Breakpoint format: <addr>[-<addr end>][=<value>][:<rwx>[:<source>]]\n"
       "                   rwx = read / write / execute flags\n"
       "                   source = cpu, smp, vram, oam, cgram, sa1, sfx");
>>>>>>> 4bf8196826a3b33efbe28f8c454f42a3b4a1c744
  #endif
}
