#include "oam-viewer.moc"
OamViewer *oamViewer;

OamObject OamObject::getObject(unsigned i) {
  OamObject obj;

  uint8_t d0 = SNES::memory::oam[(i << 2) + 0];
  uint8_t d1 = SNES::memory::oam[(i << 2) + 1];
  uint8_t d2 = SNES::memory::oam[(i << 2) + 2];
  uint8_t d3 = SNES::memory::oam[(i << 2) + 3];
  uint8_t d4 = SNES::memory::oam[512 + (i >> 2)];
  bool x    = d4 & (1 << ((i & 3) << 1));
  bool size = d4 & (2 << ((i & 3) << 1));

  switch(SNES::ppu.oam_base_size()) { default:
    case 0: obj.width = !size ?  8 : 16; obj.height = !size ?  8 : 16; break;
    case 1: obj.width = !size ?  8 : 32; obj.height = !size ?  8 : 32; break;
    case 2: obj.width = !size ?  8 : 64; obj.height = !size ?  8 : 64; break;
    case 3: obj.width = !size ? 16 : 32; obj.height = !size ? 16 : 32; break;
    case 4: obj.width = !size ? 16 : 64; obj.height = !size ? 16 : 64; break;
    case 5: obj.width = !size ? 32 : 64; obj.height = !size ? 32 : 64; break;
    case 6: obj.width = !size ? 16 : 32; obj.height = !size ? 32 : 64; break;
    case 7: obj.width = !size ? 16 : 32; obj.height = !size ? 32 : 32; break;
  }

  obj.xpos = (x << 8) + d0;
  if(obj.xpos > 256) obj.xpos = sclip<9>(obj.xpos);

  obj.ypos = d1;
  obj.character = d2;
  obj.priority = (d3 >> 4) & 3;
  obj.palette = (d3 >> 1) & 7;
  obj.hFlip = d3 & 0x80;
  obj.vFlip = d3 & 0x40;
  obj.table = d3 & 0x01;

  return obj;
}

OamViewer::OamViewer() {
  setObjectName("oam-viewer");
  setWindowTitle("Sprite Viewer");
  setGeometryString(&config().geometry.oamViewer);
  application.windowList.append(this);

  inRefreshCall = false;

  layout = new QHBoxLayout;
  layout->setAlignment(Qt::AlignLeft);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  list = new QTreeWidget;
  list->setColumnCount(8);
  list->setHeaderLabels(QStringList() << "#" << "Size" << "X" << "Y" << "Char" << "Pri" << "Pal" << "Flags");
  list->setAllColumnsShowFocus(true);
  list->setAlternatingRowColors(true);
  list->setRootIsDecorated(false);
  list->setUniformRowHeights(true);
  list->setSortingEnabled(false);
  layout->addWidget(list);

  unsigned dw = list->fontMetrics().width('0');
  list->setColumnWidth(0, dw * 4);
  list->setColumnWidth(1, dw * 8);
  list->setColumnWidth(2, dw * 6);
  list->setColumnWidth(3, dw * 6);
  list->setColumnWidth(4, dw * 6);
  list->setColumnWidth(5, dw * 6);
  list->setColumnWidth(6, dw * 6);
  list->setColumnWidth(7, dw * 6);

  for(unsigned i = 0; i < 128; i++) {
    QTreeWidgetItem *item = new QTreeWidgetItem(list);
    item->setData(0, Qt::DisplayRole, i);
    item->setData(0, Qt::UserRole, QVariant(i));
    item->setTextAlignment(0, Qt::AlignRight);
    item->setTextAlignment(1, Qt::AlignHCenter);
    item->setTextAlignment(2, Qt::AlignRight);
    item->setTextAlignment(3, Qt::AlignRight);
    item->setTextAlignment(4, Qt::AlignRight);
    item->setTextAlignment(5, Qt::AlignRight);
    item->setTextAlignment(6, Qt::AlignRight);
    item->setTextAlignment(7, Qt::AlignLeft);
  }
  list->setCurrentItem(NULL);

  list->sortItems(0, Qt::AscendingOrder);
  list->setSortingEnabled(true);


  controlLayout = new QVBoxLayout;
  controlLayout->setAlignment(Qt::AlignTop);
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);


  canvas = new OamCanvas;
  controlLayout->addWidget(canvas);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);


  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(list, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectedChanged()));
}

void OamViewer::show() {
  Window::show();
  refresh();
}

void OamViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void OamViewer::refresh() {
  // Required to prevent the occasional infinite signal call loop.
  if(inRefreshCall) return;
  inRefreshCall = true;

  int selectedRow = -1;
  if (list->currentItem()) selectedRow = list->currentIndex().row();

  list->setSortingEnabled(false);

  for(unsigned r = 0; r < list->topLevelItemCount(); r++) {
    QTreeWidgetItem *item = list->topLevelItem(r);
    unsigned i = item->data(0, Qt::UserRole).toUInt();

    OamObject obj = OamObject::getObject(i);

    string flags;
    if(obj.hFlip) flags << "V";
    if(obj.vFlip) flags << "H";

    item->setText(1, string() << obj.width << "x" << obj.height);
    item->setData(2, Qt::DisplayRole, obj.xpos);
    item->setData(3, Qt::DisplayRole, obj.ypos);
    item->setData(4, Qt::DisplayRole, obj.character + (obj.table << 8));
    item->setData(5, Qt::DisplayRole, obj.priority);
    item->setData(6, Qt::DisplayRole, obj.palette);
    item->setText(7, flags);
  }

  list->setSortingEnabled(true);

  if(selectedRow >= 0 && selectedRow < list->topLevelItemCount()) {
    list->setCurrentItem(list->topLevelItem(selectedRow));
  }
  canvas->refresh();

  inRefreshCall = false;
}

void OamViewer::onSelectedChanged() {
  QTreeWidgetItem *item = list->currentItem();

  int s = -1;
  if (item) s = item->data(0, Qt::UserRole).toUInt();

  canvas->setSelected(s);
  refresh();
}

OamCanvas::OamCanvas() {
  setFrameStyle(QFrame::Shape::Panel | QFrame::Sunken);
  setLineWidth(2);

  selected = -1;
  setScale(2);
}

void OamCanvas::paintEvent(QPaintEvent*e) {
  QFrame::paintEvent(e);

  if(!image.isNull()) {
    QPainter painter(this);
    painter.setRenderHints(0);

    unsigned x = (width() - image.width()) / 2;
    unsigned y = (width() - image.height()) / 2;
    painter.drawImage(x, y, image);
  }
}

void OamCanvas::setScale(unsigned z) {
  imageSize = 64 * z;
  setFixedSize(imageSize + frameWidth() * 2, imageSize + frameWidth() * 2);
  refresh();
}

void OamCanvas::setSelected(int s) {
  selected = s;
}

void OamCanvas::refresh() {
  if(SNES::cartridge.loaded() && selected >= 0 && selected < 128) {
    refreshImage(OamObject::getObject(selected));
  } else {
    image = QImage();
  }

  update();
}

void OamCanvas::refreshImage(const OamObject& obj) {
  QRgb palette[16];
  for(unsigned i = 0; i < 16; i++) {
    palette[i] = rgbFromCgram(128 + obj.palette * 16 + i);
  }

  QImage buffer(obj.width, obj.height, QImage::Format_RGB32);
  const uint8_t *objTileset = SNES::memory::vram.data() + SNES::ppu.oam_tile_addr(obj.table);

  for(unsigned ty = 0; ty < obj.height / 8; ty++) {
    for(unsigned tx = 0; tx < obj.width / 8; tx++) {
      unsigned cx = (obj.character + tx) & 0x00f;
      unsigned cy = (obj.character / 16) + ty;
      const uint8_t* tile = objTileset + cy * 512 + cx * 32;

      for(unsigned py = 0; py < 8; py++) {
        QRgb* dest = ((QRgb*)buffer.scanLine(ty * 8 + py)) + tx * 8;

        uint8_t d0 = tile[ 0];
        uint8_t d1 = tile[ 1];
        uint8_t d2 = tile[16];
        uint8_t d3 = tile[17];
        for(unsigned px = 0; px < 8; px++) {
          uint8_t pixel = 0;
          pixel |= (d0 & (0x80 >> px)) ? 1 : 0;
          pixel |= (d1 & (0x80 >> px)) ? 2 : 0;
          pixel |= (d2 & (0x80 >> px)) ? 4 : 0;
          pixel |= (d3 & (0x80 >> px)) ? 8 : 0;
          dest[px] = palette[pixel];
        }
        tile += 2;
      }
    }
  }

  int zoom = imageSize / maximumOamBaseSize();
  int xScale = obj.vFlip ? -zoom : zoom;
  int yScale = obj.hFlip ? -zoom : zoom;

  image = buffer.transformed(QTransform::fromScale(xScale, yScale), Qt::FastTransformation);
}

unsigned OamCanvas::maximumOamBaseSize() {
  switch(SNES::ppu.oam_base_size()) {
    case 0: return 16;
    case 1: return 32;
    case 2: return 64;
    case 3: return 32;
    case 4: return 64;
    case 5: return 64;
    case 6: return 64;
    case 7: return 32;
  }
  return 16;
}
