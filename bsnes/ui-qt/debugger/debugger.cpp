#include "../ui-base.hpp"

#if defined(DEBUGGER)

#include <nall/snes/cpu.hpp>
#include <nall/snes/smp.hpp>

#include "debugger.moc"
Debugger *debugger;

#include "tracer.cpp"

#include "disassembler/symbols/symbol_map.cpp"
#include "disassembler/symbols/symbol_map_cpu.hpp"
#include "disassembler/symbols/symbol_map_smp.hpp"

#include "disassembler/processor/processor.cpp"
#include "disassembler/processor/common_processor.cpp"
#include "disassembler/processor/cpu_processor.cpp"
#include "disassembler/processor/smp_processor.cpp"
#include "disassembler/processor/sfx_processor.cpp"

#include "registeredit.cpp"
#include "debuggerview.cpp"

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

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  menu = new QMenuBar;
  layout->setMenuBar(menu);

  menu_tools = menu->addMenu("&Tools");
  menu_tools_breakpoint = menu_tools->addAction("&Breakpoint Editor ...");
  menu_tools_memory = menu_tools->addAction("&Memory Editor ...");
  menu_tools_propertiesViewer = menu_tools->addAction("&Properties Viewer ...");

  menu_ppu = menu->addMenu("&S-PPU");
  menu_ppu_tileViewer = menu_ppu->addAction("&Tile Viewer ...");
  menu_ppu_tilemapViewer = menu_ppu->addAction("Tile&map Viewer ...");
  menu_ppu_oamViewer = menu_ppu->addAction("&Sprite Viewer ...");
  menu_ppu_cgramViewer = menu_ppu->addAction("&Palette Viewer ...");

  menu_misc = menu->addMenu("&Misc");
  menu_misc_clear = menu_misc->addAction("&Clear Console");
  menu_misc->addSeparator();
  menu_misc_cacheUsage = menu_misc->addAction("Cache &memory usage table to disk");
  menu_misc_cacheUsage->setCheckable(true);
  menu_misc_cacheUsage->setChecked(config().debugger.cacheUsageToDisk);
  menu_misc_saveBreakpoints = menu_misc->addAction("Save &breakpoints to disk between sessions");
  menu_misc_saveBreakpoints->setCheckable(true);
  menu_misc_saveBreakpoints->setChecked(config().debugger.saveBreakpoints);
  menu_misc_saveSymbols = menu_misc->addAction("Save &symbols to disk between sessions");
  menu_misc_saveSymbols->setCheckable(true);
  menu_misc_saveSymbols->setChecked(config().debugger.saveSymbols);
  menu_misc_loadDefaultSymbols = menu_misc->addAction("Load &default symbols if none are saved");
  menu_misc_loadDefaultSymbols->setCheckable(true);
  menu_misc_loadDefaultSymbols->setChecked(config().debugger.loadDefaultSymbols);
  menu_misc_showHClocks = menu_misc->addAction("Show &H-position in clocks instead of dots");
  menu_misc_showHClocks->setCheckable(true);
  menu_misc_showHClocks->setChecked(config().debugger.showHClocks);

  tracer = new Tracer;
  breakpointEditor = new BreakpointEditor;
  propertiesViewer = new PropertiesViewer;
  tileViewer = new TileViewer;
  tilemapViewer = new TilemapViewer;
  oamViewer = new OamViewer;
  cgramViewer = new CgramViewer;

  registerEditCPU = new RegisterEditCPU(SNES::cpu);
  registerEditSMP = new RegisterEditSMP;
  registerEditSA1 = new RegisterEditCPU(SNES::sa1);
  registerEditSFX = new RegisterEditSFX;

  QToolBar *toolBar = new QToolBar;
  layout->addWidget(toolBar);

  consoleLayout = new QSplitter(Qt::Vertical);
  layout->addWidget(consoleLayout);

  symbolsCPU = new SymbolMap();
  symbolsSA1 = new SymbolMap();
  symbolsSMP = new SymbolMap();
  symbolsSFX = new SymbolMap();
  if (config().debugger.loadDefaultSymbols) {
    symbolsCPU->loadFromString(DEFAULT_SYMBOL_MAP_CPU);
    symbolsSMP->loadFromString(DEFAULT_SYMBOL_MAP_SMP);
  }
  
  debugCPU = new DebuggerView(registerEditCPU, new CpuDisasmProcessor(CpuDisasmProcessor::CPU, symbolsCPU), true);
  debugSMP = new DebuggerView(registerEditSMP, new SmpDisasmProcessor(symbolsSMP));
  debugSA1 = new DebuggerView(registerEditSA1, new CpuDisasmProcessor(CpuDisasmProcessor::SA1, symbolsSA1));
  debugSFX = new DebuggerView(registerEditSFX, new SfxDisasmProcessor(symbolsSFX));

  QTabWidget *editTabs = new QTabWidget;
  editTabs->addTab(debugCPU, "CPU");
  editTabs->addTab(debugSMP, "SMP");
  editTabs->addTab(debugSA1, "SA-1");
  editTabs->addTab(debugSFX, "SuperFX");
  editTabs->setTabPosition(QTabWidget::North);
  editTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  consoleLayout->addWidget(editTabs);

  console = new QTextEdit;
  console->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  console->setReadOnly(true);
  console->setFont(QFont(Style::Monospace));
  console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  console->setMinimumWidth((98 + 4) * console->fontMetrics().width(' '));
  console->setMinimumHeight((6 + 1) * console->fontMetrics().height());
  consoleLayout->addWidget(console);

  runBreak = new QToolButton;
  runBreak->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  runBreak->setDefaultAction(new QAction(QIcon(":16x16/dbg-break.png"), "Break", this));
  runBreak->setFixedWidth(runBreak->minimumSizeHint().width());
  runBreak->defaultAction()->setToolTip("Pause/resume execution (F5)");
  runBreak->defaultAction()->setShortcut(Qt::Key_F5);
  toolBar->addWidget(runBreak);

  stepInstruction = new QToolButton;
  stepInstruction->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  stepInstruction->setDefaultAction(new QAction(QIcon(":16x16/dbg-step.png"), "Step", this));
  stepInstruction->defaultAction()->setToolTip("Step through current instruction (F6)");
  stepInstruction->defaultAction()->setShortcut(Qt::Key_F6);
  toolBar->addWidget(stepInstruction);

  stepOver = new QToolButton;
  stepOver->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  stepOver->setDefaultAction(new QAction(QIcon(":16x16/dbg-step-over.png"), "Over", this));
  stepOver->defaultAction()->setToolTip("Step over current instruction (F7)");
  stepOver->defaultAction()->setShortcut(Qt::Key_F7);
  toolBar->addWidget(stepOver);

  stepOut = new QToolButton;
  stepOut->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  stepOut->setDefaultAction(new QAction(QIcon(":16x16/dbg-step-out.png"), "Out", this));
  stepOut->defaultAction()->setToolTip("Step out of current routine (F8)");
  stepOut->defaultAction()->setShortcut(Qt::Key_F8);
  toolBar->addWidget(stepOut);

  toolBar->addSeparator();

  stepToVBlank = new QToolButton;
  stepToVBlank->setDefaultAction(new QAction("Run to VBlank", this));
  stepToVBlank->defaultAction()->setToolTip("Resume execution until next vertical blank (F9)");
  stepToVBlank->defaultAction()->setShortcut(Qt::Key_F9);
  toolBar->addWidget(stepToVBlank);

  stepToHBlank = new QToolButton;
  stepToHBlank->setDefaultAction(new QAction("Run to HBlank", this));
  stepToHBlank->defaultAction()->setToolTip("Resume execution until next horizontal blank (F10)");
  stepToHBlank->defaultAction()->setShortcut(Qt::Key_F10);
  toolBar->addWidget(stepToHBlank);
  
  stepToNMI = new QToolButton;
  stepToNMI->setDefaultAction(new QAction("Run to NMI", this));
  stepToNMI->defaultAction()->setToolTip("Resume execution until next NMI (F11)");
  stepToNMI->defaultAction()->setShortcut(Qt::Key_F11);
  toolBar->addWidget(stepToNMI);

  stepToIRQ = new QToolButton;
  stepToIRQ->setDefaultAction(new QAction("Run to IRQ", this));
  stepToIRQ->defaultAction()->setToolTip("Resume execution until next IRQ (F12)");
  stepToIRQ->defaultAction()->setShortcut(Qt::Key_F12);
  toolBar->addWidget(stepToIRQ);

  toolBar->addSeparator();

  traceMask = new QToolButton;
  traceMask->setDefaultAction(new QAction("Enable trace mask", this));
  traceMask->defaultAction()->setCheckable(true);
  toolBar->addWidget(traceMask);

  connect(menu_tools_breakpoint, SIGNAL(triggered()), breakpointEditor, SLOT(show()));
  connect(menu_tools_memory, SIGNAL(triggered()), this, SLOT(createMemoryEditor()));
  connect(menu_tools_propertiesViewer, SIGNAL(triggered()), propertiesViewer, SLOT(show()));

  connect(menu_ppu_tileViewer, SIGNAL(triggered()), tileViewer, SLOT(show()));
  connect(menu_ppu_tilemapViewer, SIGNAL(triggered()), tilemapViewer, SLOT(show()));
  connect(menu_ppu_oamViewer, SIGNAL(triggered()), oamViewer, SLOT(show()));
  connect(menu_ppu_cgramViewer, SIGNAL(triggered()), cgramViewer, SLOT(show()));

  connect(menu_misc_clear, SIGNAL(triggered()), this, SLOT(clear()));
  connect(menu_misc_cacheUsage, SIGNAL(triggered()), this, SLOT(synchronize()));
  connect(menu_misc_saveBreakpoints, SIGNAL(triggered()), this, SLOT(synchronize()));
  connect(menu_misc_loadDefaultSymbols, SIGNAL(triggered()), this, SLOT(synchronize()));
  connect(menu_misc_saveSymbols, SIGNAL(triggered()), this, SLOT(synchronize()));
  connect(menu_misc_showHClocks, SIGNAL(triggered()), this, SLOT(synchronize()));

  connect(runBreak->defaultAction(), SIGNAL(triggered()), this, SLOT(toggleRunStatus()));

  connect(stepInstruction->defaultAction(), SIGNAL(triggered()), this, SLOT(stepAction()));
  connect(stepOver->defaultAction(), SIGNAL(triggered()), this, SLOT(stepOverAction()));
  connect(stepOut->defaultAction(), SIGNAL(triggered()), this, SLOT(stepOutAction()));
  connect(stepToVBlank->defaultAction(), SIGNAL(triggered()), this, SLOT(stepToVBlankAction()));
  connect(stepToHBlank->defaultAction(), SIGNAL(triggered()), this, SLOT(stepToHBlankAction()));
  connect(stepToNMI->defaultAction(), SIGNAL(triggered()), this, SLOT(stepToNMIAction()));
  connect(stepToIRQ->defaultAction(), SIGNAL(triggered()), this, SLOT(stepToIRQAction()));

  connect(debugCPU, SIGNAL(synchronized()), this, SLOT(synchronize()));
  connect(debugSMP, SIGNAL(synchronized()), this, SLOT(synchronize()));
  connect(debugSA1, SIGNAL(synchronized()), this, SLOT(synchronize()));
  connect(debugSFX, SIGNAL(synchronized()), this, SLOT(synchronize()));

  connect(debugCPU, SIGNAL(traceStateChanged(int)), tracer, SLOT(setCpuTraceState(int)));
  connect(debugSMP, SIGNAL(traceStateChanged(int)), tracer, SLOT(setSmpTraceState(int)));
  connect(debugSA1, SIGNAL(traceStateChanged(int)), tracer, SLOT(setSa1TraceState(int)));
  connect(debugSFX, SIGNAL(traceStateChanged(int)), tracer, SLOT(setSfxTraceState(int)));
  connect(traceMask->defaultAction(), SIGNAL(toggled(bool)), tracer, SLOT(setTraceMaskState(bool)));

  frameCounter = 0;
  synchronize();
  resize(855, 745);

  QTimer *updateTimer = new QTimer(this);
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(frameTick()));
  updateTimer->start(15);
}

void Debugger::createMemoryEditor() {
  MemoryEditor *editor = new MemoryEditor();
  editor->show();
}

void Debugger::modifySystemState(unsigned state) {
  string usagefile = filepath(nall::basename(cartridge.fileName), config().path.data);
  string bpfile = usagefile;
  string symfile = usagefile;

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
    } else {
      SNES::cpuAnalyst.performFullAnalysis();
    }
    
    symbolsCPU->reset();
    symbolsSMP->reset();
    symbolsSA1->reset();
    symbolsSFX->reset();
    
    if (!symbolsCPU->loadFromFile(nall::basename(symfile), ".cpu.sym") &&
        !symbolsCPU->loadFromFile(nall::basename(symfile), ".sym") &&
        config().debugger.loadDefaultSymbols) {
      symbolsCPU->loadFromString(DEFAULT_SYMBOL_MAP_CPU);
    }
    if (!symbolsSMP->loadFromFile(nall::basename(symfile), ".smp.sym") &&
        config().debugger.loadDefaultSymbols) {
      symbolsSMP->loadFromString(DEFAULT_SYMBOL_MAP_SMP);
    }
    if (SNES::cartridge.has_sa1())
      symbolsSA1->loadFromFile(nall::basename(symfile), ".sa1.sym");
    if (SNES::cartridge.has_superfx())
      symbolsSFX->loadFromFile(nall::basename(symfile), ".sfx.sym");
    
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
    
    if(config().debugger.saveSymbols) {
      symbolsCPU->saveToFile(nall::basename(symfile), ".cpu.sym");
      symbolsSMP->saveToFile(nall::basename(symfile), ".smp.sym");
      if (SNES::cartridge.has_sa1())
        symbolsSA1->saveToFile(nall::basename(symfile), ".sa1.sym");
      if (SNES::cartridge.has_superfx())
        symbolsSFX->saveToFile(nall::basename(symfile), ".sfx.sym");
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

  runBreak->defaultAction()->setIcon(active ? QIcon(":16x16/dbg-run.png") : QIcon(":16x16/dbg-break.png"));
  runBreak->defaultAction()->setText(active ? "Run" : "Break");

  bool stepEnabled = SNES::cartridge.loaded() && active &&
                     (debugCPU->stepProcessor->isChecked() || debugSMP->stepProcessor->isChecked() ||
                      debugSA1->stepProcessor->isChecked() || debugSFX->stepProcessor->isChecked());
  bool stepOtherEnabled = stepEnabled && (debugCPU->stepProcessor->isChecked() + debugSMP->stepProcessor->isChecked() +
                                          debugSA1->stepProcessor->isChecked() + debugSFX->stepProcessor->isChecked() == 1)
                          && !debugSFX->stepProcessor->isChecked(); // TODO: implement this for superfx
  bool stepHVBEnabled = stepEnabled && debugCPU->stepProcessor->isChecked();
  bool stepInterruptEnabled = stepOtherEnabled && (debugCPU->stepProcessor->isChecked() || debugSA1->stepProcessor->isChecked());

  stepInstruction->setEnabled(stepEnabled);
  stepOver->setEnabled(stepOtherEnabled);
  stepOut->setEnabled(stepOtherEnabled);
  stepToVBlank->setEnabled(stepHVBEnabled);
  stepToHBlank->setEnabled(stepHVBEnabled);
  stepToNMI->setEnabled(stepInterruptEnabled);
  stepToIRQ->setEnabled(stepInterruptEnabled);
  
  config().debugger.cacheUsageToDisk = menu_misc_cacheUsage->isChecked();
  config().debugger.saveBreakpoints = menu_misc_saveBreakpoints->isChecked();
  config().debugger.loadDefaultSymbols = menu_misc_loadDefaultSymbols->isChecked();
  config().debugger.saveSymbols = menu_misc_saveSymbols->isChecked();
  config().debugger.showHClocks = menu_misc_showHClocks->isChecked();
  
  // todo: factor in whether or not cartridge actually contains SA1/SuperFX
  SNES::debugger.step_cpu = application.debug && debugCPU->stepProcessor->isChecked();
  SNES::debugger.step_smp = application.debug && debugSMP->stepProcessor->isChecked();
  SNES::debugger.step_sa1 = application.debug && debugSA1->stepProcessor->isChecked();
  SNES::debugger.step_sfx = application.debug && debugSFX->stepProcessor->isChecked();

  if(!active) {
    registerEditCPU->setEnabled(false);
    registerEditSMP->setEnabled(false);
    registerEditSA1->setEnabled(false);
    registerEditSFX->setEnabled(false);
  }
  QVectorIterator<MemoryEditor*> i(memoryEditors);
  while (i.hasNext()) {
    i.next()->synchronize();
  }
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

void Debugger::stepToVBlankAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepToVBlank;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
  synchronize();
  switchWindow();
}

void Debugger::stepToHBlankAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepToHBlank;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
  synchronize();
  switchWindow();
}

void Debugger::stepToNMIAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepToNMI;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
  synchronize();
  switchWindow();
}

void Debugger::stepToIRQAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepToIRQ;
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
      
      if (n == SNES::Debugger::SoftBreakCPU)
        echo(string() << "Software breakpoint hit (CPU).<br>");
      else if (n == SNES::Debugger::SoftBreakSA1)
        echo(string() << "Software breakpoint hit (SA-1).<br>");
      else if (n < SNES::debugger.breakpoint.size())
        echo(string() << "Breakpoint " << n << " hit (" << SNES::debugger.breakpoint[n].counter << ").<br>");
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
        debugCPU->refresh(SNES::cpu.opcode_pc);
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
        debugSA1->refresh(SNES::sa1.opcode_pc);
        registerEditSA1->setEnabled(true);
        break;
      }

      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::APURAM) {
        SNES::debugger.step_smp = true;
        SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        debugSMP->refresh(SNES::smp.opcode_pc);
        registerEditSMP->setEnabled(true);
        break;
      }

      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::SFXBus) {
        SNES::debugger.step_sfx = true;
        SNES::superfx.disassemble_opcode(t, SNES::superfx.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        debugSFX->refresh(SNES::superfx.opcode_pc);
        registerEditSFX->setEnabled(true);
        break;
      }
    } break;

    case SNES::Debugger::BreakEvent::CPUStep: {
      SNES::cpu.disassemble_opcode(t, SNES::cpu.opcode_pc, config().debugger.showHClocks);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#0000a0'>" << s << "</font><br>");
      debugCPU->refresh(SNES::cpu.opcode_pc);
      registerEditCPU->setEnabled(true);
    } break;

    case SNES::Debugger::BreakEvent::SMPStep: {
      SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#a00000'>" << s << "</font><br>");
      debugSMP->refresh(SNES::smp.opcode_pc);
      registerEditSMP->setEnabled(true);
    } break;

    case SNES::Debugger::BreakEvent::SA1Step: {
      SNES::sa1.disassemble_opcode(t, SNES::sa1.opcode_pc, config().debugger.showHClocks);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#008000'>" << s << "</font><br>");
      debugSA1->refresh(SNES::sa1.opcode_pc);
      registerEditSA1->setEnabled(true);
    } break;

    case SNES::Debugger::BreakEvent::SFXStep: {
      SNES::superfx.disassemble_opcode(t, SNES::superfx.opcode_pc, true);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#008000'>" << s << "</font><br>");
      debugSFX->refresh(SNES::superfx.opcode_pc);
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
    QVectorIterator<MemoryEditor*> i(memoryEditors);
    while (i.hasNext()) {
      i.next()->autoUpdate();
    }
  }
  
  frameCounter = frame;
}

void Debugger::autoUpdate() {
  QVectorIterator<MemoryEditor*> i(memoryEditors);
  while (i.hasNext()) {
    i.next()->synchronize();
  }
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
