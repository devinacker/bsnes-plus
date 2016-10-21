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

  //TODO: Properly expose these values to the debugger
  //TODO: Add second sprite table address

  vramAddrItems[0] = new VramAddrItem("BG1:", "BG1 Name Base Address");
  vramAddrItems[1] = new VramAddrItem("BG2:", "BG2 Name Base Address");
  vramAddrItems[2] = new VramAddrItem("BG3:", "BG3 Name Base Address");
  vramAddrItems[3] = new VramAddrItem("BG4:", "BG4 Name Base Address");
  vramAddrItems[4] = new VramAddrItem("OAM:", "OAM Name Base Address");

  for (unsigned i = 0; i < N_MAP_ITEMS; i++) {
    sidebarLayout->addWidget(vramAddrItems[i]);
  }

  mainLayout->addStretch();

  scrollArea = new QScrollArea;
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setMinimumHeight(300);
  mainLayout->addWidget(scrollArea);

  canvas = new VramCanvas;
  scrollArea->setWidget(canvas);

  vramInfo = new QLabel;
  layout->addWidget(vramInfo);

  canvas->setDepth2bpp();
  depth2bpp->setChecked(true);

  zoom->setCurrentIndex(1);
  zoomChanged(1);

  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoom,       SIGNAL(currentIndexChanged(int)), this, SLOT(zoomChanged(int)));
  connect(depth2bpp,  SIGNAL(pressed()), canvas, SLOT(setDepth2bpp()));
  connect(depth4bpp,  SIGNAL(pressed()), canvas, SLOT(setDepth4bpp()));
  connect(depth8bpp,  SIGNAL(pressed()), canvas, SLOT(setDepth8bpp()));
  connect(depthMode7, SIGNAL(pressed()), canvas, SLOT(setDepthMode7()));
  connect(canvas,     SIGNAL(infoChanged(unsigned)), this, SLOT(displayInfo(unsigned)));

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
  canvas->refresh();

  for (unsigned i = 0; i < N_MAP_ITEMS; i++) {
    vramAddrItems[i]->refresh();
  }
}

void VramViewer::zoomChanged(int index) {
  unsigned z = zoom->itemData(index).toUInt();
  if(z == 0) z = 1;

  canvas->setZoom(z);
  scrollArea->setMinimumWidth(canvas->width() + scrollArea->verticalScrollBar()->width() * 2);
}

void VramViewer::gotoAddress(unsigned vram_addr) {
  int y = canvas->tileYPos(vram_addr);

  scrollArea->verticalScrollBar()->setValue(y);
}

VramCanvas::VramCanvas() {
  image = new QImage(128, 2048, QImage::Format_RGB32);
  image->fill(0x800000);

  zoom = 1;
  setDepth2bpp();
}

void VramCanvas::refresh() {
  image->fill(0x800000);
  if(SNES::cartridge.loaded()) {
    const uint8_t *source = SNES::memory::vram.data();

    if(bpp == 2) refresh2bpp (source);
    if(bpp == 4) refresh4bpp (source);
    if(bpp == 8) refresh8bpp (source);
    if(bpp == 7) refreshMode7(source);
  }
  update();
}

void VramCanvas::refresh2bpp(const uint8_t *source) {
  uint32_t *dest = (uint32_t*)image->bits();

  for(unsigned ty = 0; ty < 256; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        uint8_t d0 = source[0];
        uint8_t d1 = source[1];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          pixel *= 0x55;
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
        }
        source += 2;
      }
    }
  }
}

void VramCanvas::refresh4bpp(const uint8_t *source) {
  uint32_t *dest = (uint32_t*)image->bits();

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
          pixel *= 0x11;
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
        }
        source += 2;
      }
      source += 16;
    }
  }
}

void VramCanvas::refresh8bpp(const uint8_t *source) {
  uint32_t *dest = (uint32_t*)image->bits();

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
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
        }
        source += 2;
      }
      source += 48;
    }
  }
}

void VramCanvas::refreshMode7(const uint8_t *source) {
  uint32_t *dest = (uint32_t*)image->bits();

  for(unsigned ty = 0; ty < 16; ty++) {
    for(unsigned tx = 0; tx < 16; tx++) {
      for(unsigned py = 0; py < 8; py++) {
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = source[1];
          dest[(ty * 8 + py) * 128 + (tx * 8 + px)] = (pixel << 16) + (pixel << 8) + pixel;
          source += 2;
        }
      }
    }
  }
}

void VramCanvas::setDepth2bpp()  { bpp = 2; updateWidgetSize(); }
void VramCanvas::setDepth4bpp()  { bpp = 4; updateWidgetSize(); }
void VramCanvas::setDepth8bpp()  { bpp = 8; updateWidgetSize(); }
void VramCanvas::setDepthMode7() { bpp = 7; updateWidgetSize(); }

void VramCanvas::setZoom(unsigned newZoom) {
    zoom = newZoom;

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
  painter.scale(zoom, zoom);
  painter.drawImage(0, 0, *image);
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

VramAddrItem::VramAddrItem(const QString& text, const string& propertyName) {
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

  propertyId = 0;
  while(true) {
    string name, value;

    propertyId++;
    bool s = SNES::ppu.property(propertyId, name, value);

    if(s == false || name == propertyName) break;
  }

  connect(gotoButton, SIGNAL(released()), this, SLOT(onGotoPressed()));
}

void VramAddrItem::refresh() {
  string label, value;
  SNES::ppu.property(propertyId, label, value);

  address->setText(value);
}

void VramAddrItem::onGotoPressed() {
  gotoPressed(hex(address->text().toUtf8().data()));
}
