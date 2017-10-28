#include "vram-viewer.moc"
VramViewer *vramViewer;

VramViewer::VramViewer() {
  setObjectName("vram-viewer");
  setWindowTitle("Video RAM Viewer");
  setGeometryString(&config().geometry.vramViewer);
  application.windowList.append(this);

  layout = new QVBoxLayout;
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  layout->setAlignment(Qt::AlignCenter);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  controlLayout = new QHBoxLayout;
  controlLayout->setSizeConstraint(QLayout::SetMinimumSize);
  controlLayout->setAlignment(Qt::AlignRight);
  layout->addLayout(controlLayout);

  mainLayout = new QHBoxLayout;
  mainLayout->setSizeConstraint(QLayout::SetMinimumSize);
  mainLayout->setAlignment(Qt::AlignTop);
  layout->addLayout(mainLayout);

  sidebarLayout = new QVBoxLayout;
  sidebarLayout->setAlignment(Qt::AlignTop);
  sidebarLayout->setSizeConstraint(QLayout::SetFixedSize);
  mainLayout->addLayout(sidebarLayout);

  depth2bpp = new QRadioButton("2 BPP");
  controlLayout->addWidget(depth2bpp);

  depth4bpp = new QRadioButton("4 BPP");
  controlLayout->addWidget(depth4bpp);

  depth8bpp = new QRadioButton("8 BPP");
  controlLayout->addWidget(depth8bpp);

  depthMode7 = new QRadioButton("Mode 7");
  controlLayout->addWidget(depthMode7);

  zoom = new QComboBox;
  zoom->addItem("1x", QVariant(1));
  zoom->addItem("3x", QVariant(3));
  zoom->addItem("5x", QVariant(5));
  zoom->addItem("7x", QVariant(7));
  zoom->addItem("9x", QVariant(9));
  controlLayout->addWidget(zoom);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);

  baseAddrLabel = new QLabel("Base Tile Addresses:");
  sidebarLayout->addWidget(baseAddrLabel);

  vramAddrItems[0] = new VramAddrItem("BG1:", 0);
  vramAddrItems[1] = new VramAddrItem("BG2:", 1);
  vramAddrItems[2] = new VramAddrItem("BG3:", 2);
  vramAddrItems[3] = new VramAddrItem("BG4:", 3);
  vramAddrItems[4] = new VramAddrItem("OAM1:", 4);
  vramAddrItems[5] = new VramAddrItem("OAM2:", 5);

  for (unsigned i = 0; i < N_MAP_ITEMS; i++) {
    sidebarLayout->addWidget(vramAddrItems[i]);
  }

  sidebarLayout->addSpacing(Style::WidgetSpacing);

  useCgram = new QCheckBox("Use CGRAM");
  sidebarLayout->addWidget(useCgram);

  cgramWidget = new CgramWidget;
  cgramWidget->setScale(12);
  sidebarLayout->addWidget(cgramWidget);

  mainLayout->addStretch();

  scrollArea = new QScrollArea;
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setMinimumHeight(300);
  mainLayout->addWidget(scrollArea);

  vramCanvas = new VramCanvas;
  scrollArea->setWidget(vramCanvas);

  vramInfo = new QLabel;
  layout->addWidget(vramInfo);

  depth2bpp->setChecked(true);
  onDepthChanged();

  cgramWidget->setSelected(-1);
  vramCanvas->setSelectedColor(0);

  zoom->setCurrentIndex(1);
  zoomChanged(1);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(vramCanvas, SIGNAL(infoChanged(unsigned)), this, SLOT(displayInfo(unsigned)));

  connect(zoom,       SIGNAL(currentIndexChanged(int)), this, SLOT(zoomChanged(int)));
  connect(depth2bpp,  SIGNAL(clicked()), this, SLOT(onDepthChanged()));
  connect(depth4bpp,  SIGNAL(clicked()), this, SLOT(onDepthChanged()));
  connect(depth8bpp,  SIGNAL(clicked()), this, SLOT(onDepthChanged()));
  connect(depthMode7, SIGNAL(clicked()), this, SLOT(onDepthChanged()));

  connect(useCgram,    SIGNAL(clicked()), this, SLOT(onUseCgramPressed()));
  connect(cgramWidget, SIGNAL(selectedChanged()), this, SLOT(onCgramSelectedChanged()));

  for (unsigned i = 0; i < N_MAP_ITEMS; i++) {
    connect(vramAddrItems[i], SIGNAL(gotoPressed(unsigned)), this, SLOT(gotoAddress(unsigned)));
  }
}

void VramViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void VramViewer::show() {
  Window::show();
  refresh();
}

void VramViewer::refresh() {
  vramCanvas->refresh();
  cgramWidget->refresh();

  for(unsigned i = 0; i < N_MAP_ITEMS; i++) {
    vramAddrItems[i]->refresh();
  }
}

void VramViewer::zoomChanged(int index) {
  unsigned z = zoom->itemData(index).toUInt();
  if(z == 0) z = 1;

  vramCanvas->setZoom(z);

  int scrollWidth = vramCanvas->width() + scrollArea->verticalScrollBar()->width() * 2;
  scrollArea->setMinimumWidth(scrollWidth);

  int mainWidth = scrollWidth + sidebarLayout->minimumSize().width();
  int controlWidth = controlLayout->minimumSize().width();

  setFixedWidth(max(mainWidth, controlWidth) + Style::WidgetSpacing * 3);
}

void VramViewer::gotoAddress(unsigned vram_addr) {
  int y = vramCanvas->tileYPos(vram_addr);

  scrollArea->verticalScrollBar()->setValue(y);
}

void VramViewer::onDepthChanged() {
  if(depth2bpp->isChecked())  { vramCanvas->setDepth2bpp();  cgramWidget->setPaletteBpp(2); }
  if(depth4bpp->isChecked())  { vramCanvas->setDepth4bpp();  cgramWidget->setPaletteBpp(4); }
  if(depth8bpp->isChecked())  { vramCanvas->setDepth8bpp();  cgramWidget->setPaletteBpp(8); }
  if(depthMode7->isChecked()) { vramCanvas->setDepthMode7(); cgramWidget->setPaletteBpp(8); }
}

void VramViewer::onUseCgramPressed() {
  if(useCgram->isChecked()) {
    if(!cgramWidget->hasSelected()) cgramWidget->setSelected(0);
  } else {
    cgramWidget->selectNone();
  }
}

void VramViewer::onCgramSelectedChanged() {
  if(cgramWidget->hasSelected()) {
    vramCanvas->setUseCgram(true);
    useCgram->setChecked(true);
    vramCanvas->setSelectedColor(cgramWidget->selectedPalette());
  } else {
    useCgram->setChecked(false);
    vramCanvas->setUseCgram(false);
  }

  refresh();
}

VramCanvas::VramCanvas() {
  image = QImage(128, 2048, QImage::Format_RGB32);
  image.fill(0x800000);

  zoom = 1;
  selectedColor = 0;
  useCgram = false;
  setDepth2bpp();
  refreshScaledImage();
}

void VramCanvas::setDepth2bpp()  { bpp = 2; buildDefaultPalette(); updateWidgetSize(); }
void VramCanvas::setDepth4bpp()  { bpp = 4; buildDefaultPalette(); updateWidgetSize(); }
void VramCanvas::setDepth8bpp()  { bpp = 8; buildDefaultPalette(); updateWidgetSize(); }
void VramCanvas::setDepthMode7() { bpp = 7; buildDefaultPalette(); updateWidgetSize(); }

void VramCanvas::setUseCgram(bool b) {
  useCgram = b;
  if (!useCgram) buildDefaultPalette();
}

void VramCanvas::setSelectedColor(unsigned c) {
  if (c > 255) c = 0;
  selectedColor = c;
}

void VramCanvas::buildDefaultPalette() {
  unsigned nColors = 1 << bpp;
  if(bpp == 7) nColors = 256;

  uint8_t delta = 1;
  if(bpp == 2) delta = 0x55;
  if(bpp == 4) delta = 0x11;

  uint8_t pixel = 0;

  for(unsigned i = 0; i < nColors; i++) {
    palette[i] = qRgb(pixel, pixel, pixel);
    pixel += delta;
  }
}

void VramCanvas::buildCgramPalette() {
  unsigned nColors = 1 << bpp;
  if(bpp == 7) nColors = 256;

  unsigned start = selectedColor & (0xff - nColors + 1);
  assert(start + nColors < 256);

  for(unsigned i = 0; i < nColors; i++) {
    palette[i] = rgbFromCgram(start + i);
  }
}

void VramCanvas::refresh() {
  if(SNES::cartridge.loaded()) {
    if(useCgram) buildCgramPalette();

    const uint8_t *source = SNES::memory::vram.data();

    if(bpp == 2) refresh2bpp (source);
    if(bpp == 4) refresh4bpp (source);
    if(bpp == 8) refresh8bpp (source);
    if(bpp == 7) refreshMode7(source);
  }

  refreshScaledImage();
  update();
}

void VramCanvas::refresh2bpp(const uint8_t *source) {
  QRgb* dest = (QRgb*)image.bits();

  for(unsigned ty = 0; ty < 256; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[0];
        uint8_t d1 = source[1];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = palette[pixel];
        }
        source += 2;
      }
    }
  }
}

void VramCanvas::refresh4bpp(const uint8_t *source) {
  QRgb* dest = (QRgb*)image.bits();

  for(unsigned ty = 0; ty < 128; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[ 0];
        uint8_t d1 = source[ 1];
        uint8_t d2 = source[16];
        uint8_t d3 = source[17];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          pixel |= (d2 & (0x80 >> px)) ? 4 : 0;
          pixel |= (d3 & (0x80 >> px)) ? 8 : 0;
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = palette[pixel];
        }
        source += 2;
      }
      source += 16;
    }
  }
}

void VramCanvas::refresh8bpp(const uint8_t *source) {
  QRgb* dest = (QRgb*)image.bits();

  for(unsigned ty = 0; ty < 64; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[ 0];
        uint8_t d1 = source[ 1];
        uint8_t d2 = source[16];
        uint8_t d3 = source[17];
        uint8_t d4 = source[32];
        uint8_t d5 = source[33];
        uint8_t d6 = source[48];
        uint8_t d7 = source[49];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 0x01 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 0x02 : 0;
          pixel |= (d2 & (0x80 >> px)) ? 0x04 : 0;
          pixel |= (d3 & (0x80 >> px)) ? 0x08 : 0;
          pixel |= (d4 & (0x80 >> px)) ? 0x10 : 0;
          pixel |= (d5 & (0x80 >> px)) ? 0x20 : 0;
          pixel |= (d6 & (0x80 >> px)) ? 0x40 : 0;
          pixel |= (d7 & (0x80 >> px)) ? 0x80 : 0;
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = palette[pixel];
        }
        source += 2;
      }
      source += 48;
    }
  }
}

void VramCanvas::refreshMode7(const uint8_t *source) {
  QRgb* dest = (QRgb*)image.bits();

  for(unsigned ty = 0; ty < 16; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = source[1];
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = palette[pixel];
          source += 2;
        }
      }
    }
  }
}

void VramCanvas::refreshScaledImage() {
  scaledImage = image.scaled(image.width() * zoom, image.height() * zoom, Qt::IgnoreAspectRatio, Qt::FastTransformation);
}

void VramCanvas::setZoom(unsigned newZoom) {
    zoom = newZoom;
    refreshScaledImage();

    updateWidgetSize();
}

void VramCanvas::updateWidgetSize() {
    if (bpp == 2) setFixedSize(128 * zoom, 2048 * zoom);
    if (bpp == 4) setFixedSize(128 * zoom, 1024 * zoom);
    if (bpp == 8) setFixedSize(128 * zoom,  512 * zoom);
    if (bpp == 7) setFixedSize(128 * zoom,  128 * zoom);

    refresh();
}

void VramCanvas::paintEvent(QPaintEvent*) {
  QPainter painter(this);
  painter.setRenderHints(0);
  painter.drawImage(0, 0, scaledImage);
}

void VramViewer::displayInfo(unsigned vram_addr) {
	if (vram_addr <= 0xFFFF) {
		char tmp[256] = "";
		sprintf(tmp, "VRAM address: %04X", vram_addr);
	    vramInfo->setText(tmp);
	}
	else
		vramInfo->setText("");
}

void VramCanvas::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
	  unsigned column = event->x() / 8 / zoom;
	  unsigned row = event->y() / 8 / zoom;
	  unsigned tile_num = (row * 16) + column;
	  unsigned vram_addr = (tile_num * bpp * 8);
	  emit infoChanged(vram_addr);
  }
}

int VramCanvas::tileYPos(unsigned vram_addr) {
  if (bpp == 7 || vram_addr > 0xffff) return 0;

  unsigned tile = vram_addr / 8 / bpp;
  unsigned row = tile / 16;
  unsigned ypos = row * 8;
  return ypos * zoom;
}

VramAddrItem::VramAddrItem(const QString& text, unsigned num) {
  layout = new QHBoxLayout;
  layout->setAlignment(Qt::AlignLeft);
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  layout->setContentsMargins(-1, 0, -1, 0);
  setLayout(layout);

  label = new QLabel(text);
  layout->addWidget(label);

  address = new QLineEdit();
  address->setFixedWidth(9 * address->fontMetrics().width('0'));
  address->setReadOnly(true);
  layout->addWidget(address);

  gotoButton = new QToolButton;
  gotoButton->setDefaultAction(new QAction("goto", this));
  layout->addWidget(gotoButton);

  index = num;

  connect(gotoButton, SIGNAL(released()), this, SLOT(onGotoPressed()));
}

void VramAddrItem::refresh() {
  unsigned value = 0;
  switch (index) {
  case 0: value = SNES::ppu.bg_tile_addr(0); break;
  case 1: value = SNES::ppu.bg_tile_addr(1); break;
  case 2: value = SNES::ppu.bg_tile_addr(2); break;
  case 3: value = SNES::ppu.bg_tile_addr(3); break;
  case 4: value = SNES::ppu.oam_tile_addr(0); break;
  case 5: value = SNES::ppu.oam_tile_addr(1); break;
  }

  address->setText(string("0x", hex<4>(value)));
}

void VramAddrItem::onGotoPressed() {
  gotoPressed(hex(address->text().toUtf8().data()));
}
