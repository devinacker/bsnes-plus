class Debugger : public Window {
  Q_OBJECT

public:
  QMenuBar *menu;
  QMenu *menu_tools;
  QAction *menu_tools_disassembler;
  QAction *menu_tools_breakpoint;
  QAction *menu_tools_memory;
  QAction *menu_tools_propertiesViewer;
  QMenu *menu_ppu;
  QAction *menu_ppu_vramViewer;
  QAction *menu_ppu_tileViewer;
  QAction *menu_ppu_tilemapViewer;
  QAction *menu_ppu_oamViewer;
  QAction *menu_ppu_cgramViewer;
  QMenu *menu_misc;
  QAction *menu_misc_clear;
  QAction *menu_misc_options;

  QHBoxLayout *layout;
  QTextEdit *console;
  QVBoxLayout *consoleLayout;
  QVBoxLayout *controlLayout;
  QHBoxLayout *commandLayout;
  QToolButton *runBreak;
  QToolButton *stepInstruction;
  QToolButton *stepOver;
  QToolButton *stepOut;
  QCheckBox *stepCPU;
  QCheckBox *stepSMP;
  QCheckBox *stepSA1;
  QCheckBox *stepSFX;
  QCheckBox *traceCPU;
  QCheckBox *traceSMP;
  QCheckBox *traceSA1;
  QCheckBox *traceSFX;
  QCheckBox *traceMask;
  QWidget *spacer;

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

private:
  inline void switchWindow();

  unsigned frameCounter;
};

extern Debugger *debugger;
