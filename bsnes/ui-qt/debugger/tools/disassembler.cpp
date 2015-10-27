#include "disassembler.moc"
DisasmWidget *cpuDisassembler;
DisasmWidget *smpDisassembler;
DisasmWidget *sa1Disassembler;
DisasmWidget *sfxDisassembler;
Disassembler *disassembler;

DisasmWidget::DisasmWidget() {
  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  view = new QTextEdit;
  view->setReadOnly(true);
  view->setFont(QFont(Style::Monospace));
  view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  view->setMinimumHeight((25 + 1) * view->fontMetrics().height());
  layout->addWidget(view);
}

Disassembler::Disassembler() {
  setObjectName("disassembler");
  setWindowTitle("Disassembler");
  setGeometryString(&config().geometry.disassembler);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  cpuDisassembler = new DisasmWidget;
  smpDisassembler = new DisasmWidget;
  sa1Disassembler = new DisasmWidget;
  sfxDisassembler = new DisasmWidget;

  tab = new QTabWidget;
  tab->addTab(cpuDisassembler, "S-CPU");
  tab->addTab(smpDisassembler, "S-SMP");
  tab->addTab(sa1Disassembler, "SA-1");
  tab->addTab(sfxDisassembler, "SuperFX");
  layout->addWidget(tab);
}

void Disassembler::refresh(Source source, unsigned addr) {
  uint8_t *usage;
  unsigned mask;
  if(source == CPU) { usage = SNES::cpu.usage; mask = (1 << 24) - 1; }
  if(source == SMP) { usage = SNES::smp.usage; mask = (1 << 16) - 1; }
  if(source == SA1) { usage = SNES::sa1.usage; mask = (1 << 24) - 1; }
  if(source == SFX) { usage = SNES::superfx.usage; mask = (1 << 23) - 1; }

  int line[25];
  for(unsigned i = 0; i < 25; i++) line[i] = -1;

  line[12] = addr;

  for(signed index = 11; index >= 0; index--) {
    int base = line[index + 1];
    if(base == -1) break;

    for(unsigned i = 1; i <= 4; i++) {
      if(usage[(base - i) & mask] & 0x10) {
        line[index] = base - i;
        break;
      }
    }
  }

  for(signed index = 13; index <= 24; index++) {
    int base = line[index - 1];
    if(base == -1) break;

    for(unsigned i = 1; i <= 4; i++) {
      if(usage[(base + i) & mask] & 0x10) {
        line[index] = base + i;
        break;
      }
    }
  }

  string output;
  for(unsigned i = 0; i < 25; i++) {
    if(i < 12) output << "<font color='#0000a0'>";
    else if(i == 12) output << "<font color='#00a000'>";
    else output << "<font color='#a00000'>";

    if(line[i] == -1) {
      output << "...";
    } else {
      char t[256];
      if(source == CPU) { SNES::cpu.disassemble_opcode(t, line[i]); t[20] = 0; }
      if(source == SMP) { SNES::smp.disassemble_opcode(t, line[i]); t[23] = 0; }
      if(source == SA1) { SNES::sa1.disassemble_opcode(t, line[i]); t[20] = 0; }
      if(source == SFX) { SNES::superfx.disassemble_opcode(t, line[i]); t[25] = 0; }
      string text = rtrim(t);
      text.replace(" ", "&nbsp;");
      output << text;
    }

    output << "</font>";
    if(i != 24) output << "<br>";
  }

  if(source == CPU) cpuDisassembler->view->setHtml(output);
  if(source == SMP) smpDisassembler->view->setHtml(output);
  if(source == SA1) sa1Disassembler->view->setHtml(output);
  if(source == SFX) sfxDisassembler->view->setHtml(output);
}
