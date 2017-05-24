#include "cgram-viewer.moc"
CgramViewer *cgramViewer;

CgramViewer::CgramViewer() {
  setObjectName("cgram-viewer");
  setWindowTitle("Palette Viewer");
  setGeometryString(&config().geometry.cgramViewer);
  application.windowList.append(this);

  layout = new QHBoxLayout;
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setMargin(UIStyle::WindowMargin);
  layout->setSpacing(UIStyle::WidgetSpacing);
  setLayout(layout);

  cgramWidget = new CgramWidget;
  layout->addWidget(cgramWidget);

  controlLayout = new QVBoxLayout;
  controlLayout->setAlignment(Qt::AlignTop);
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);
  controlLayout->addSpacing(UIStyle::WidgetSpacing);

  colorInfo = new QLabel;
  controlLayout->addWidget(colorInfo);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(cgramWidget, SIGNAL(selectedChanged()), this, SLOT(refresh()));
}

void CgramViewer::show() {
  Window::show();
  refresh();
}

void CgramViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void CgramViewer::refresh() {
  cgramWidget->refresh();

  string text;

  if (cgramWidget->hasSelected()) {
    unsigned selected = cgramWidget->selectedColor();

    uint16_t color = SNES::memory::cgram[selected * 2 + 0];
    color |= SNES::memory::cgram[selected * 2 + 1] << 8;

    uint8_t r = (color >>  0) & 31;
    uint8_t g = (color >>  5) & 31;
    uint8_t b = (color >> 10) & 31;

    text << "<table>";
    text << "<tr><td>Index:</td><td>" << selected << "</td></tr>";
    text << "<tr><td>Value:</td><td>0x" << hex<4>(color) << "</td></tr>";
    text << "<tr><td>Red:</td><td>" << (unsigned)r << "</td></tr>";
    text << "<tr><td>Green:</td><td>" << (unsigned)g << "</td></tr>";
    text << "<tr><td>Blue:</td><td>" << (unsigned)b << "</td></tr>";
    text << "</table>";
  }
  colorInfo->setText(text);
}
