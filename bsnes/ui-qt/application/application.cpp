#include "application.moc"
VideoDisplay display;
Application application;

#include "init.cpp"
#include "arguments.cpp"

VideoDisplay::VideoDisplay() {
  outputWidth = 0;
  outputHeight = 0;

  cropLeft = 0;
  cropTop = 0;
  cropRight = 0;
  cropBottom = 0;
}

void Application::initPaths(const char *basename) {
  char temp[PATH_MAX];

  if(realpath(basename, temp)) {
    //remove program name
    strtr(temp, "\\", "/");
    for(signed i = strlen(temp) - 1; i >= 0; i--) {
      if(temp[i] == '/') {
        temp[i] = 0;
        break;
      }
    }

    if(strend(temp, "/") == false) strcat(temp, "/");
    config().path.base = temp;
  } else {
    config().path.base = "";
  }

  if(userpath(temp)) {
    strtr(temp, "\\", "/");
    if(strend(temp, "/") == false) strcat(temp, "/");
    config().path.user = temp;
  } else {
    config().path.user = "";
  }

  char cwd[PATH_MAX];
  config().path.startup = getcwd(cwd);
}

bool Application::singleUserMode() {
  //check if config file exists in executable directory (single-user mode)
  string temp = string() << config().path.base << "bsnes-qt.cfg";
  return file::exists(temp);
}

void Application::locateFile(string &filename, bool createDataDirectory) {
  string temp;

  if(singleUserMode()) {
    temp = string() << config().path.base << filename;
  } else {
    //if not, use user data path (multi-user mode)
    temp = config().path.user;
    temp << ".bsnes";
    if(createDataDirectory) mkdir(temp, 0755);  //ensure directory exists
    temp << "/" << filename;
  }

  filename = temp;
}

void Application::loadCartridge(const string &filename) {
    if(striend(filename, ".bs")) {
      if(config().path.bsx == "") {
        loaderWindow->loadBsxCartridge("", filename);
      } else {
        cartridge.loadBsx(config().path.bsx, filename);
      }
      
    } else if(striend(filename, ".st")) { 
      if(config().path.st == "") {
        loaderWindow->loadSufamiTurboCartridge("", filename, "");
      } else {
        cartridge.loadSufamiTurbo(config().path.st, filename, "");
      }
      
    } else if(striend(filename, ".gb") || striend(filename, ".sgb") || striend(filename, ".gbc")) {
      if(config().path.sgb == "") {
        loaderWindow->loadSuperGameBoyCartridge("", filename);
      } else {
        cartridge.loadSuperGameBoy(config().path.sgb, filename);
      }
      
    } else if(striend(filename, ".spc")) {
      cartridge.loadSpc(filename);
      
    } else if(striend(filename, ".snsf") || striend(filename, ".minisnsf"))  {
      cartridge.loadSnsf(filename);
      
    } else {
      cartridge.loadNormal(filename);
      
    }
}

int Application::main(int &argc, char **argv) {
  app = new App(argc, argv);
  #if !defined(PLATFORM_WIN)
    #if defined(PLATFORM_OSX)
    app->setWindowIcon(QIcon(":/bsnes_512.png"));
    #else
    app->setWindowIcon(QIcon(":/bsnes.png"));
    #endif
  #else
  //Windows port uses 256x256 icon from resource file
  CoInitialize(0);
  utf8_args(argc, argv);
  #endif

  initPaths(argv[0]);
  locateFile(configFilename = "bsnes-qt.cfg", true);
  locateFile(styleSheetFilename = "style.qss");
  locateFile(cheatsFilename = "cheats.xml");

  string customStylesheet;
  if(customStylesheet.readfile(styleSheetFilename) == true) {
    app->setStyleSheet((const char*)customStylesheet);
  } else {
    app->setStyleSheet(defaultStylesheet);
  }

  if(!QFile::exists(cheatsFilename)) {
    QFile::copy(":/cheats.xml", cheatsFilename);
  }

  config().load(configFilename);
  mapper().bind();
  init();
  SNES::system.init(&interface);
  mainWindow->system_loadSpecial_superGameBoy->setVisible(SNES::supergameboy.opened());

  parseArguments();

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(run()));
  timer->start(0);
  app->exec();

  //QbWindow::close() saves window geometry for next run
  for(unsigned i = 0; i < windowList.size(); i++) {
    windowList[i]->close();
  }

  cartridge.unload();
  config().save(configFilename);
  return 0;
}

void Application::run() {
  if(terminate == true) {
    timer->stop();
    app->quit();
    return;
  }
  
  utility.updateSystemState();
  mapper().poll();

  if(config().input.focusPolicy == Configuration::Input::FocusPolicyPauseEmulation) {
    bool active = mainWindow->isActive();
    if(!autopause && !active) {
      autopause = true;
      audio.clear();
    } else if(autopause && active) {
      autopause = false;
    }
  } else {
    autopause = false;
  }

  if(SNES::cartridge.loaded() && !pause && !autopause && (!debug || debugrun)) {
    SNES::system.run();
    #if defined(DEBUGGER)
    if(SNES::debugger.break_event != SNES::Debugger::BreakEvent::None) {
      debug = true;
      debugrun = false;
      debugger->synchronize();
      debugger->event();
      SNES::debugger.break_event = SNES::Debugger::BreakEvent::None;
    }
    #endif
    if(frameAdvance && SNES::scheduler.exit_reason() == SNES::Scheduler::ExitReason::FrameEvent) {
      pause = true;
    }
  } else {
    usleep(20 * 1000);
    if (frameAdvance) {
      audio.clear();
      frameAdvance = false;
    }
  }

  clock_t currentTime = clock();
  autosaveTime += currentTime - clockTime;
  screensaverTime += currentTime - clockTime;
  clockTime = currentTime;

  if(autosaveTime >= CLOCKS_PER_SEC * 60) {
    //auto-save RAM once per minute in case of emulator crash
    autosaveTime = 0;
    if(config().system.autoSaveMemory == true) cartridge.saveMemory();
  }

  if(screensaverTime >= CLOCKS_PER_SEC * 30) {
    //supress screen saver every 30 seconds so it will not trigger during gameplay
    screensaverTime = 0;
    supressScreenSaver();
  }
}

Application::Application() : timer(0) {
  terminate    = false;
  power        = false;
  frameAdvance = false;
  pause        = false;
  autopause    = false;
  debug        = false;
  debugrun     = false;

  clockTime       = clock();
  autosaveTime    = 0;
  screensaverTime = 0;
  
  loadType = SNES::Cartridge::Mode::Normal;
}

Application::~Application() {
  delete timer;

  //deleting (QApplication)app will segfault the application upon exit
  //delete app;
}
