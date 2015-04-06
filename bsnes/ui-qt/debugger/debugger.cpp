#include "../ui-base.hpp"

#if defined(DEBUGGER)

#include "debugger.moc"
Debugger *debugger;

#include "tracer.cpp"

#include "tools/disassembler.cpp"
#include "tools/breakpoint.cpp"
#include "tools/memory.cpp"
#include "tools/properties.cpp"

#include "ppu/vram-viewer.cpp"
#include "ppu/oam-viewer.cpp"
#include "ppu/cgram-viewer.cpp"

#include "misc/debugger-options.cpp"

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
  menu_ppu_vramViewer = menu_ppu->addAction("Video RAM Viewer ...");
  menu_ppu_oamViewer = menu_ppu->addAction("Sprite Viewer ...");
  menu_ppu_cgramViewer = menu_ppu->addAction("Palette Viewer ...");

  menu_misc = menu->addMenu("Misc");
  menu_misc_clear = menu_misc->addAction("Clear Console");
  menu_misc_options = menu_misc->addAction("Options ...");

  console = new QTextEdit;
  console->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  console->setReadOnly(true);
  console->setFont(QFont(Style::Monospace));
  console->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  console->setMinimumWidth((92 + 4) * console->fontMetrics().width(' '));
  console->setMinimumHeight((25 + 1) * console->fontMetrics().height());
  layout->addWidget(console);

  controlLayout = new QVBoxLayout;
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  commandLayout = new QHBoxLayout;
  controlLayout->addLayout(commandLayout);

  // TODO: icons/hotkeys instead of text
  runBreak = new QToolButton;
  runBreak->setText("Brk");
  commandLayout->addWidget(runBreak);
  commandLayout->addSpacing(Style::WidgetSpacing);

  stepInstruction = new QToolButton;
  stepInstruction->setText("Step");
  commandLayout->addWidget(stepInstruction);

  stepOver = new QToolButton;
  stepOver->setText("Over");
  commandLayout->addWidget(stepOver);

  stepOut = new QToolButton;
  stepOut->setText("Out");
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
  vramViewer = new VramViewer;
  oamViewer = new OamViewer;
  cgramViewer = new CgramViewer;
  debuggerOptions = new DebuggerOptions;

  connect(menu_tools_disassembler, SIGNAL(triggered()), disassembler, SLOT(show()));
  connect(menu_tools_breakpoint, SIGNAL(triggered()), breakpointEditor, SLOT(show()));
  connect(menu_tools_memory, SIGNAL(triggered()), memoryEditor, SLOT(show()));
  connect(menu_tools_propertiesViewer, SIGNAL(triggered()), propertiesViewer, SLOT(show()));

  connect(menu_ppu_vramViewer, SIGNAL(triggered()), vramViewer, SLOT(show()));
  connect(menu_ppu_oamViewer, SIGNAL(triggered()), oamViewer, SLOT(show()));
  connect(menu_ppu_cgramViewer, SIGNAL(triggered()), cgramViewer, SLOT(show()));

  connect(menu_misc_clear, SIGNAL(triggered()), this, SLOT(clear()));
  connect(menu_misc_options, SIGNAL(triggered()), debuggerOptions, SLOT(show()));

  connect(runBreak, SIGNAL(released()), this, SLOT(toggleRunStatus()));
  
  connect(stepInstruction, SIGNAL(released()), this, SLOT(stepAction()));
  connect(stepOver, SIGNAL(released()), this, SLOT(stepOverAction()));
  connect(stepOut, SIGNAL(released()), this, SLOT(stepOutAction()));
  
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
}

void Debugger::modifySystemState(unsigned state) {
  // TODO: include cartridge ROM/RAM and expansion usage in this
  string usagefile = filepath(nall::basename(cartridge.fileName), config().path.data);
  string cartusagefile = usagefile;
  usagefile << "-usage.bin";
  file fp;

  if(state == Utility::LoadCartridge) {
    memset(SNES::cpu.cart_usage, 0x00, 1 << 24);
    if(config().debugger.cacheUsageToDisk && fp.open(usagefile, file::mode::read)) {
      fp.read(SNES::cpu.usage, 1 << 24);
      fp.read(SNES::smp.usage, 1 << 16);
      fp.close();
      
      for (unsigned i = 0; i < 1 << 24; i++) {
        int offset = SNES::cartridge.rom_offset(i);
        if (offset >= 0) SNES::cpu.cart_usage[offset] |= SNES::cpu.usage[i];
      }
    } else {
      memset(SNES::cpu.usage, 0x00, 1 << 24);
      memset(SNES::smp.usage, 0x00, 1 << 16);
    }
  }

  if(state == Utility::UnloadCartridge) {
    if(config().debugger.cacheUsageToDisk && fp.open(usagefile, file::mode::write)) {
      fp.write(SNES::cpu.usage, 1 << 24);
      fp.write(SNES::smp.usage, 1 << 16);
      fp.close();
    }
  }
}

void Debugger::synchronize() {
  runBreak->setText(application.debug ? "Run" : "Break");
  bool stepEnabled = SNES::cartridge.loaded() && application.debug && 
                     (stepCPU->isChecked() || stepSMP->isChecked() || 
                      stepSA1->isChecked() || stepSFX->isChecked());
  bool stepOtherEnabled = stepEnabled && (stepCPU->isChecked() + stepSMP->isChecked() + 
                                          stepSA1->isChecked() + stepSFX->isChecked() == 1)
                          && !stepSFX->isChecked(); // TODO: implement this for superfx
  
  stepInstruction->setEnabled(stepEnabled);
  stepOver->setEnabled(stepOtherEnabled);
  stepOut->setEnabled(stepOtherEnabled);
  SNES::debugger.step_cpu = application.debug && stepCPU->isChecked();
  SNES::debugger.step_smp = application.debug && stepSMP->isChecked();
  SNES::debugger.step_sa1 = application.debug && stepSA1->isChecked();
  SNES::debugger.step_sfx = application.debug && stepSFX->isChecked();

  memoryEditor->synchronize();
}

void Debugger::echo(const char *message) {
  console->moveCursor(QTextCursor::End);
  console->insertHtml(message);
}

void Debugger::clear() {
  console->setHtml("");
}

void Debugger::toggleRunStatus() {
  application.debug = !application.debug;
  if(!application.debug) application.debugrun = false;
  else audio.clear();
  synchronize();
  
  // TODO: disassemble current address when breaking (if any are selected)
}

void Debugger::stepAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepInto;
  application.debugrun = true;
}

void Debugger::stepOverAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepOver;
  SNES::debugger.step_over_new = true;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
}

void Debugger::stepOutAction() {
  SNES::debugger.step_type = SNES::Debugger::StepType::StepOut;
  SNES::debugger.call_count = 0;
  
  application.debugrun = true;
}

void Debugger::event() {
  char t[256];

  switch(SNES::debugger.break_event) {
    case SNES::Debugger::BreakEvent::BreakpointHit: {
      unsigned n = SNES::debugger.breakpoint_hit;
      echo(string() << "Breakpoint " << n << " hit (" << SNES::debugger.breakpoint[n].counter << ").<br>");

      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::CPUBus) {
        SNES::debugger.step_cpu = true;
        SNES::cpu.disassemble_opcode(t, SNES::cpu.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::CPU, SNES::cpu.opcode_pc);
      }

      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::APURAM) {
        SNES::debugger.step_smp = true;
        SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << t << "</font><br>");
        disassembler->refresh(Disassembler::SMP, SNES::smp.opcode_pc);
      }
      
      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::SA1Bus) {
        SNES::debugger.step_sa1 = true;
        SNES::sa1.disassemble_opcode(t, SNES::sa1.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::SA1, SNES::sa1.opcode_pc);
      }
      
      if(SNES::debugger.breakpoint[n].source == SNES::Debugger::Breakpoint::Source::SFXBus) {
        SNES::debugger.step_sfx = true;
        SNES::superfx.disassemble_opcode(t, SNES::superfx.opcode_pc);
        string s = t;
        s.replace(" ", "&nbsp;");
        echo(string() << "<font color='#a000a0'>" << s << "</font><br>");
        disassembler->refresh(Disassembler::SFX, SNES::superfx.opcode_pc);
      }
    } break;

    case SNES::Debugger::BreakEvent::CPUStep: {
      SNES::cpu.disassemble_opcode(t, SNES::cpu.opcode_pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#0000a0'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::CPU, SNES::cpu.opcode_pc);
    } break;

    case SNES::Debugger::BreakEvent::SMPStep: {
      SNES::smp.disassemble_opcode(t, SNES::smp.opcode_pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#a00000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SMP, SNES::smp.opcode_pc);
    } break;
    
    case SNES::Debugger::BreakEvent::SA1Step: {
      SNES::sa1.disassemble_opcode(t, SNES::sa1.opcode_pc);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#008000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SA1, SNES::sa1.opcode_pc);
    } break;
    
    case SNES::Debugger::BreakEvent::SFXStep: {
      SNES::superfx.disassemble_opcode(t, SNES::superfx.opcode_pc, true);
      string s = t;
      s.replace(" ", "&nbsp;");
      echo(string() << "<font color='#008000'>" << s << "</font><br>");
      disassembler->refresh(Disassembler::SFX, SNES::superfx.opcode_pc);
    } break;
  }

  audio.clear();
  autoUpdate();
}

//called once every time a video frame is rendered, used to update "auto refresh" tool windows
void Debugger::frameTick() {
  if(++frameCounter >= (SNES::system.region() == SNES::System::Region::NTSC ? 60 : 50)) {
    frameCounter = 0;
    autoUpdate();
  }
}

void Debugger::autoUpdate() {
  memoryEditor->autoUpdate();
  propertiesViewer->autoUpdate();
  vramViewer->autoUpdate();
  oamViewer->autoUpdate();
  cgramViewer->autoUpdate();
}

#endif
