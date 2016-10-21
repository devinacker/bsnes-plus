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

  scrollArea = new QScrollArea;
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setMinimumHeight(300);
  layout->addWidget(scrollArea);

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
}

void VramViewer::zoomChanged(int index) {
  unsigned z = zoom->itemData(index).toUInt();
  if(z == 0) z = 1;

  canvas->setZoom(z);
  scrollArea->setMinimumWidth(canvas->width() + scrollArea->verticalScrollBar()->width() * 2);
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
