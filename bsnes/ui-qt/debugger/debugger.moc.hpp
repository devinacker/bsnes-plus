class Debugger : public Window {
  Q_OBJECT

public:
  QMenuBar *menu;
  QMenu *menu_tools;
  QAction *menu_tools_breakpoint;
  QAction *menu_tools_memory;
  QAction *menu_tools_propertiesViewer;
  QMenu *menu_ppu;
  QAction *menu_ppu_tileViewer;
  QAction *menu_ppu_tilemapViewer;
  QAction *menu_ppu_oamViewer;
  QAction *menu_ppu_cgramViewer;
  QMenu *menu_misc;
  QAction *menu_misc_clear;
  QAction *menu_misc_cacheUsage;
  QAction *menu_misc_saveBreakpoints;
  QAction *menu_misc_loadDefaultSymbols;
  QAction *menu_misc_saveSymbols;
  QAction *menu_misc_showHClocks;
  QAction *menu_misc_options;

  QVBoxLayout *layout;
  QSplitter *consoleLayout;
  QTextEdit *console;
  QToolBar *toolBar;
  QToolButton *runBreak;
  QToolButton *stepInstruction;
  QToolButton *stepOver;
  QToolButton *stepOut;
  QToolButton *stepToVBlank;
  QToolButton *stepToHBlank;
  QToolButton *stepToNMI;
  QToolButton *stepToIRQ;
  QToolButton *traceMask;

  class DebuggerView *debugCPU;
  class DebuggerView *debugSFX;
  class DebuggerView *debugSA1;
  class DebuggerView *debugSMP;

  class SymbolMap *symbolsCPU;
  class SymbolMap *symbolsSFX;
  class SymbolMap *symbolsSA1;
  class SymbolMap *symbolsSMP;

  void modifySystemState(unsigned);
  void echo(const char *message);
  void event();
  void autoUpdate();
  Debugger();

public slots:
  void clear();
  void synchronize();
  void frameTick();

  void toggleRunStatus();
  void stepAction();
  void stepOverAction();
  void stepOutAction();
  void stepToVBlankAction();
  void stepToHBlankAction();
  void stepToNMIAction();
  void stepToIRQAction();
  void createMemoryEditor();

private:
  inline void switchWindow();

  unsigned frameCounter;
  string defaultSymbolsCPU;
  string defaultSymbolsSMP;
};

extern Debugger *debugger;
