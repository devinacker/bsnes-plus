#include "../ui-base.hpp"

#if defined(DEBUGGER)

#include "debugger.moc"
Debugger *debugger;

#include "tracer.cpp"

#include "registeredit.cpp"

#include "tools/disassembler.cpp"
#include "tools/breakpoint.cpp"
#include "tools/memory.cpp"
#include "tools/properties.cpp"

#include "ppu/base-renderer.cpp"
#include "ppu/tile-renderer.cpp"
#include "ppu/tilemap-renderer.cpp"

#include "ppu/cgram-widget.cpp"
#include "ppu/image-grid-widget.cpp"

#include "ppu/oam-data-model.cpp"
#include "ppu/oam-graphics-scene.cpp"

#include "ppu/tile-viewer.cpp"
#include "ppu/tilemap-viewer.cpp"
#include "ppu/oam-viewer.cpp"
#include "ppu/cgram-viewer.cpp"

Debugger::Debugger() {
  setObjectName("debugger");
  setWindowTitle("Debugger");
  setGeometryString(&config().geometry.debugger);
  application.windowList.append(this);

  layout = new QHBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  menu = new QMenuBar;
  layout->setMenuBar(menu);

  menu_tools = menu->addMenu("Tools");
  menu_tools_disassembler = menu_tools->addAction("Disassembler ...");
  menu_tools_breakpoint = menu_tools->addAction("Breakpoint Editor ...");
  menu_tools_memory = menu_tools->addAction("Memory Editor ...");
  menu_tools_propertiesViewer = menu_tools->addAction("Properties Viewer ...");

  menu_ppu = menu->addMenu("S-PPU");
  menu_ppu_tileViewer = menu_ppu->addAction("Tile Viewer ...");
  menu_ppu_tilemapViewer = menu_ppu->addAction("Tilemap Viewer ...");
  menu_ppu_oamViewer = menu_ppu->addAction("Sprite Viewer ...");
  menu_ppu_cgramViewer = menu_ppu->addAction("Palette Viewer ...");

  menu_misc = menu->addMenu("Misc");
  menu_misc_clear = menu_misc->addAction("Clear Console");
  menu_misc->addSeparator();
  menu_misc_cacheUsage = menu_misc->addAction("Cache memory usage table to disk");
  menu_misc_cacheUsage->setCheckable(true);
  menu_misc_cacheUsage->setChecked(config().debugger.cacheUsageToDisk);
  menu_misc_saveBreakpoints = menu_misc->addAction("Save breakpoints to disk between sessions");
  menu_misc_saveBreakpoints->setCheckable(true);
  menu_misc_saveBreakpoints->setChecked(config().debugger.saveBreakpoints);
  menu_misc_showHClocks = menu_misc->addAction("Show H-position in clocks instead of dots");
  menu_misc_showHClocks->setCheckable(true);
  menu_misc_showHClocks->setChecked(config().debugger.showHClocks);

  consoleLayout = new QVBoxLayout;
  consoleLayout->setSpacing(0);
  layout->addLayout(consoleLayout);
  
  console = new QTextEdit;
  console->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  console->setReadOnly(true);
  console->setFont(QFont(Style::Monospace));
  console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  console->setMinimumWidth((98 + 4) * console->fontMetrics().width(' '));
  console->setMinimumHeight((25 + 1) * console->fontMetrics().height());
  consoleLayout->addWidget(console);

  QTabWidget *editTabs = new QTabWidget;
  editTabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  registerEditCPU = new RegisterEditCPU(SNES::cpu);
  editTabs->addTab(registerEditCPU, "CPU Registers");
  registerEditSMP = new RegisterEditSMP;
  editTabs->addTab(registerEditSMP, "SMP Registers");
  registerEditSA1 = new RegisterEditCPU(SNES::sa1);
  editTabs->addTab(registerEditSA1, "SA-1 Registers");
  registerEditSFX = new RegisterEditSFX;
  editTabs->addTab(registerEditSFX, "SuperFX Registers");
  editTabs->setTabPosition(QTabWidget::South);
  consoleLayout->addWidget(editTabs);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  commandLayout = new QHBoxLayout;
  controlLayout->addLayout(commandLayout);

  // TODO: icons instead of text
  runBreak = new QToolButton;
  runBreak->setDefaultAction(new QAction("Break", this));
  runBreak->setFixedWidth(runBreak->minimumSizeHint().width());
  runBreak->defaultAction()->setToolTip("Pause/resume execution (F5)");
  runBreak->defaultAction()->setShortcut(Qt::Key_F5);
  commandLayout->addWidget(runBreak);
  commandLayout->addSpacing(Style::WidgetSpacing);

  stepInstruction = new QToolButton;
  stepInstruction->setDefaultAction(new QAction("Step", this));
  stepInstruction->defaultAction()->setToolTip("Step through current instruction (F6)");
  stepInstruction->defaultAction()->setShortcut(Qt::Key_F6);
  commandLayout->addWidget(stepInstruction);

  stepOver = new QToolButton;
  stepOver->setDefaultAction(new QAction("Over", this));
  stepOver->defaultAction()->setToolTip("Step over current instruction (F7)");
  stepOver->defaultAction()->setShortcut(Qt::Key_F7);
  commandLayout->addWidget(stepOver);

  stepOut = new QToolButton;
  stepOut->setDefaultAction(new QAction("Out", this));
  stepOut->defaultAction()->setToolTip("Step out of current routine (F8)");
  stepOut->defaultAction()->setShortcut(Qt::Key_F8);
  commandLayout->addWidget(stepOut);

  controlLayout->addSpacing(Style::WidgetSpacing);

  stepCPU = new QCheckBox("Step S-CPU");
  stepCPU->setChecked(true);
  controlLayout->addWidget(stepCPU);

  stepSMP = new QCheckBox("Step S-SMP");
  controlLayout->addWidget(stepSMP);

  stepSA1 = new QCheckBox("Step SA-1");
  controlLayout->addWidget(stepSA1);

  stepSFX = new QCheckBox("Step SuperFX");
  controlLayout->addWidget(stepSFX);
  
  controlLayout->addSpacing(Style::WidgetSpacing);

  traceCPU = new QCheckBox("Trace S-CPU opcodes");
  controlLayout->addWidget(traceCPU);

  traceSMP = new QCheckBox("Trace S-SMP opcodes");
  controlLayout->addWidget(traceSMP);
  
  traceSA1 = new QCheckBox("Trace SA-1 opcodes");
  controlLayout->addWidget(traceSA1);
  
  traceSFX = new QCheckBox("Trace SuperFX opcodes");
  controlLayout->addWidget(traceSFX);

  traceMask = new QCheckBox("Enable trace mask");
  controlLayout->addWidget(traceMask);

  spacer = new QWidget;
  spacer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  controlLayout->addWidget(spacer);

  tracer = new Tracer;
  disassembler = new Disassembler;
  breakpointEditor = new BreakpointEditor;
  memoryEditor = new MemoryEditor;
  propertiesViewer = new PropertiesViewer;
  tileViewer = new TileViewer;
  tilemapViewer = new TilemapViewer;
  oamViewer = new OamViewer;
  cgramViewer = new CgramViewer;

  connect(menu_tools_disassembler, SIGNAL(triggered()), disassembler, SLOT(show()));
  connect(menu_tools_breakpoint, SIGNAL(triggered()), breakpointEditor, SLOT(show()));
  connect(menu_tools_memory, SIGNAL(triggered()), memoryEditor, SLOT(show()));
  connect(menu_tools_propertiesViewer, SIGNAL(triggered()), propertiesViewer, SLOT(show()));

  connect(menu_ppu_tileViewer, SIGNAL(triggered()), tileViewer, SLOT(show()));
  connect(menu_ppu_tilemapViewer, SIGNAL(triggered()), tilemapViewer, SLOT(show()));
  connect(menu_ppu_oamViewer, SIGNAL(triggered()), oamViewer, SLOT(show()));
  connect(menu_ppu_cgramViewer, SIGNAL(triggered()), cgramViewer, SLOT(show()));

  connect(menu_misc_clear, SIGNAL(triggered()), this, SLOT(clear()));
  connect(menu_misc_cacheUsage, SIGNAL(triggered()), this, SLOT(synchronize()));
  connect(menu_misc_saveBreakpoints, SIGNAL(triggered()), this, SLOT(synchronize()));
  connect(menu_misc_showHClocks, SIGNAL(triggered()), this, SLOT(synchronize()));

  connect(runBreak->defaultAction(), SIGNAL(triggered()), this, SLOT(toggleRunStatus()));
  
  connect(stepInstruction->defaultAction(), SIGNAL(triggered()), this, SLOT(stepAction()));
  connect(stepOver->defaultAction(), SIGNAL(triggered()), this, SLOT(stepOverAction()));
  connect(stepOut->defaultAction(), SIGNAL(triggered()), this, SLOT(stepOutAction()));
  
  connect(stepCPU, SIGNAL(released()), this, SLOT(synchronize()));
  connect(stepSMP, SIGNAL(released()), this, SLOT(synchronize()));
  connect(stepSA1, SIGNAL(released()), this, SLOT(synchronize()));
  connect(stepSFX, SIGNAL(released()), this, SLOT(synchronize()));
  connect(traceCPU, SIGNAL(stateChanged(int)), tracer, SLOT(setCpuTraceState(int)));
  connect(traceSMP, SIGNAL(stateChanged(int)), tracer, SLOT(setSmpTraceState(int)));
  connect(traceSA1, SIGNAL(stateChanged(int)), tracer, SLOT(setSa1TraceState(int)));
  connect(traceSFX, SIGNAL(stateChanged(int)), tracer, SLOT(setSfxTraceState(int)));
  connect(traceMask, SIGNAL(stateChanged(int)), tracer, SLOT(setTraceMaskState(int)));

  frameCounter = 0;
  synchronize();
  resize(855, 425);
  
  QTimer *updateTimer = new QTimer(this);
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(frameTick()));
  updateTimer->start(15);
}

void Debugger::modifySystemState(unsigned state) {
  string usagefile = filepath(nall::basename(cartridge.fileName), config().path.data);
  string bpfile = usagefile;
  usagefile << "-usage.bin";
  bpfile << ".bp";
  file fp;

  if(state == Utility::LoadCartridge) {
    memset(SNES::cpu.cart_usage, 0x00, 1 << 24);
    
    memset(SNES::cpu.usage, 0x00, 1 << 24);
    memset(SNES::smp.usage, 0x00, 1 << 16);
    
    memset(SNES::sa1.usage, 0x00, 1 << 24);
    memset(SNES::superfx.usage, 0x00, 1 << 23);
    
    if(config().debugger.cacheUsageToDisk && fp.open(usagefile, file::mode::read)) {
      fp.read(SNES::cpu.usage, 1 << 24);
      fp.read(SNES::smp.usage, 1 << 16);
      if (SNES::cartridge.has_sa1())     fp.read(SNES::sa1.usage, 1 << 24);
      if (SNES::cartridge.has_superfx()) fp.read(SNES::superfx.usage, 1 << 23);
      fp.close();
      
      for (unsigned i = 0; i < 1 << 24; i++) {
        int offset = SNES::cartridge.rom_offset(i);
        if (offset >= 0) 
          SNES::cpu.cart_usage[offset] |= SNES::cpu.usage[i] | SNES::sa1.usage[i];
        if (offset >= 0 && i < 0x600000)
          SNES::cpu.cart_usage[offset] |= SNES::superfx.usage[i];
      }
    }
    
    string data;
    if(config().debugger.saveBreakpoints) {
      breakpointEditor->clear();
      if (data.readfile(bpfile)) {
  	    lstring line;
        data.replace("\r", "");
        line.split("\n", data);
      
        for (int i = 0; i < line.size(); i++) {
          breakpointEditor->addBreakpoint(line[i]);
        }
      }
    }
    
    tracer->resetTraceState();
  }

  if(state == Utility::UnloadCartridge) {
    if(config().debugger.cacheUsageToDisk && fp.open(usagefile, file::mode::write)) {
      fp.write(SNES::cpu.usage, 1 << 24);
      fp.write(SNES::smp.usage, 1 << 16);
      if (SNES::cartridge.has_sa1())     fp.write(SNES::sa1.usage, 1 << 24);
      if (SNES::cartridge.has_superfx()) fp.write(SNES::superfx.usage, 1 << 23);
      fp.close();
    }
    
    if(config().debugger.saveBreakpoints) {
      string data = breakpointEditor->toStrings();
      
      // don't write an empty list of breakpoints unless the file already exists
      if ((data.length() || file::exists(bpfile)) && fp.open(bpfile, file::mode::write)) {
        fp.print(data);
        fp.close();
      }
    }
  }
}

void Debugger::synchronize() {
  bool active = application.debug && !application.debugrun;

  runBreak->defaultAction()->setText(active ? "Run" : "Break");
  bool stepEnabled = SNES::cartridge.loaded() && active && 
                     (stepCPU->isChecked() || stepSMP->isChecked() || 
                      stepSA1->isChecked() || stepSFX->isChecked());
  bool stepOtherEnabled = stepEnabled && (stepCPU->isChecked() + stepSMP->isChecked() + 
                                          stepSA1->isChecked() + stepSFX->isChecked() == 1)
                          && !stepSFX->isChecked(); // TODO: implement this for superfx
  
  stepInstruction->setEnabled(stepEnabled);
  stepOver->setEnabled(stepOtherEnabled);
  stepOut->setEnabled(stepOtherEnabled);
  
  config().debugger.cacheUsageToDisk = menu_misc_cacheUsage->isChecked();
  config().debugger.saveBreakpoints = menu_misc_saveBreakpoints->isChecked();
  config().debugger.showHClocks = menu_misc_showHClocks->isChecked();
  
  // todo: factor in whether or not cartridge actually contains SA1/SuperFX
  SNES::debugger.step_cpu = application.debug && stepCPU->isChecked();
  SNES::debugger.step_smp = application.debug && stepSMP->isChecked();
  SNES::debugger.step_sa1 = application.debug && stepSA1->isChecked();
  SNES::debugger.step_sfx = application.debug && stepSFX->isChecked();

  if(!active) {
    registerEditCPU->setEnabled(false);
    registerEditSMP->setEnabled(false);
    registerEditSA1->setEnabled(false);
    registerEditSFX->setEnabled(false);
  }
  memoryEditor->synchronize();
}

void Debugger::echo(const char *message) {
  console->moveCursor(QTextCursor::End);
  console->insertHtml(QString::fromUtf8(message));
}

void Debugger::clear() {
  console->setHtml("");
}

void Debugger::switchWindow() {
  // give focus to the main window if needed so that emulation can continue
  if(config().input.focusPolicy == Configuration::Input::FocusPolicyPauseEmulation) {
    mainWindow->activateWindow();
  }
}

void Debugger::toggleRunStatus() {
  application.debug = !application.debug || application.debugrun;
  application.debugrun = false;
  if(!application.debug) {
    mainWindow->activateWindow();
  } else {
    audio.clear();
  }
  synchronize();
  
  // TODO: disassemble current address when breaking (if any are selected)
}

void Debugger::stepAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepInto;
  application.debugrun = true;
  synchronize();
  switchWindow();
}

void Debugger::stepOverAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepOver;
  SNES::debugger.step_over_new = true;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
  synchronize();
  switchWindow();
}

void Debugger::stepOutAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepOut;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
  synchronize();
  switchWindow();
}

void Debugger::event() {
  char t[256];

  registerEditCPU->setEnabled(false);
  registerEditSMP->setEnabled(false);
  registerEditSA1->setEnabled(false);
  registerEditSFX->setEnabled(false);

  switch(SNES::debugger.break_event) {
    case SNES::Debugger::BreakEvent::BreakpointHit: {
      unsigned n = SNES::debugger.breakpoint_hit;
      
      if (n < SNES::Debugger::Breakpoints)
        echo(string() << "Breakpoint " << n << " hit (" << SNES::debugger.breakpoint[n].counter << ").<br>");
      else if (n == SNES::Debugger::SoftBreakCPU)
        echo(string() << "Software breakpoint hit (CPU).<br>");
      else if (n == SNES::Debugger::SoftBreakSA1)
        echo(string() << "Software breakpoint hit (SA-1).<br>");
      else break;
        
      if(n == SNES::Debugger::SoftBreakCPU
           || SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::CPUBus
           || SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::VRAM
           || SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::OAM
           || SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::CGRAM) {
        SNES::debugger.step_cpu = true;
        SNES::cpu.disassemble_opcode(t, SNES::cpu.opcode_pc, config().debugger.showHClocks);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::CPU, SNES::cpu.opcode_pc);
        registerEditCPU->setEnabled(true);
        break;
      }

      if(n == SNES::Debugger::SoftBreakSA1
           || SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::SA1Bus) {
        SNES::debugger.step_sa1 = true;
        SNES::sa1.disassemble_opcode(t, SNES::sa1.opcode_pc, config().debugger.showHClocks);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::SA1, SNES::sa1.opcode_pc);
        registerEditSA1->setEnabled(true);
        break;
      }
      
      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::APURAM) {
        SNES::debugger.step_smp = true;
        SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::SMP, SNES::smp.opcode_pc);
        registerEditSMP->setEnabled(true);
        break;
      }
      
      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::SFXBus) {
        SNES::debugger.step_sfx = true;
        SNES::superfx.disassemble_opcode(t, SNES::superfx.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::SFX, SNES::superfx.opcode_pc);
        registerEditSFX->setEnabled(true);
        break;
      }
    } break;

    case SNES::Debugger::BreakEvent::CPUStep: {
      SNES::cpu.disassemble_opcode(t, SNES::cpu.opcode_pc, config().debugger.showHClocks);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#0000a0'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::CPU, SNES::cpu.opcode_pc);
      registerEditCPU->setEnabled(true);
    } break;

    case SNES::Debugger::BreakEvent::SMPStep: {
      SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#a00000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SMP, SNES::smp.opcode_pc);
      registerEditSMP->setEnabled(true);
    } break;
    
    case SNES::Debugger::BreakEvent::SA1Step: {
      SNES::sa1.disassemble_opcode(t, SNES::sa1.opcode_pc, config().debugger.showHClocks);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#008000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SA1, SNES::sa1.opcode_pc);
      registerEditSA1->setEnabled(true);
    } break;
    
    case SNES::Debugger::BreakEvent::SFXStep: {
      SNES::superfx.disassemble_opcode(t, SNES::superfx.opcode_pc, true);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#008000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SFX, SNES::superfx.opcode_pc);
      registerEditSFX->setEnabled(true);
    } break;
  }

  // disable speedup/slowdown since the main window isn't going to register
  // the user probably releasing the key while the debug window is active
  HotkeyInput::releaseSpeedKeys();
  
  audio.clear();
  autoUpdate();
  show();
  activateWindow();
}

// update "auto refresh" tool windows
void Debugger::frameTick() {
  unsigned frame = SNES::cpu.framecounter();
  if (frameCounter == frame) return;

  if (frame < frameCounter) {
    autoUpdate();
  } else {
    // update memory editor every time since once per second isn't very useful
    // (TODO: and PPU viewers, maybe?) 
    memoryEditor->autoUpdate();
  }
  
  frameCounter = frame;
}

void Debugger::autoUpdate() {
  memoryEditor->autoUpdate();
  propertiesViewer->autoUpdate();
  tileViewer->autoUpdate();
  tilemapViewer->autoUpdate();
  oamViewer->autoUpdate();
  cgramViewer->autoUpdate();
  
  registerEditCPU->synchronize();
  registerEditSA1->synchronize();
  registerEditSMP->synchronize();
  registerEditSFX->synchronize();
}

#endif
