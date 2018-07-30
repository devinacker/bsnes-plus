struct VideoDisplay {
  unsigned outputWidth;
  unsigned outputHeight;

  unsigned cropLeft;
  unsigned cropTop;
  unsigned cropRight;
  unsigned cropBottom;

  VideoDisplay();
};

class Application : public QObject {
  Q_OBJECT

public:
  class App : public QApplication {
  public:
    #if defined(PLATFORM_WIN)
    bool winEventFilter(MSG *msg, long *result);
    #endif

    App(int &argc, char **argv) : QApplication(argc, argv) {}
  } *app;

  QTimer *timer;

  bool terminate;  //set to true to terminate main() loop and exit emulator
  bool power;
  bool pause;
  bool frameAdvance;
  bool autopause;
  bool debug;      //debugger sets this to true when entered to suspend emulation
  bool debugrun;   //debugger sets this to true to run emulation to a debug event

  clock_t clockTime;
  clock_t autosaveTime;
  clock_t screensaverTime;

  string configFilename;
  string styleSheetFilename;
  string cheatsFilename;
  string currentRom;

  SNES::Cartridge::Mode loadType; // used for command-line loading

  array<QWidget*> windowList;

  int main(int &argc, char **argv);
  bool singleUserMode();
  void locateFile(string &filename, bool createDataDirectory = false);
  void loadCartridge(const string& filename); // used for command-line loading
  void reloadCartridge();
  void initPaths(const char *basename);
  void init();

  void printArguments();
  void parseArguments();
  bool parseArgumentSwitch(const string& arg, const string& param);

  Application();
  ~Application();

public slots:
  void run();
};

extern VideoDisplay display;
extern Application application;
