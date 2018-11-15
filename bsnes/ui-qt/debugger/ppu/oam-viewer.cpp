#include "oam-viewer.moc"
OamViewer *oamViewer;

OamViewer::OamViewer() {
  setObjectName("oam-viewer");
  setWindowTitle("Sprite Viewer");
  setGeometryString(&config().geometry.oamViewer);
  application.windowList.append(this);

  inRefreshCall = false;

  dataModel = new OamDataModel(this);

  proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(dataModel);
  proxyModel->setSortRole(OamDataModel::SortRole);
  proxyModel->setDynamicSortFilter(true);

  layout = new QHBoxLayout;
  layout->setAlignment(Qt::AlignLeft);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  treeView = new QTreeView;
  treeView->setModel(proxyModel);
  treeView->setAllColumnsShowFocus(true);
  treeView->setAlternatingRowColors(true);
  treeView->setRootIsDecorated(false);
  treeView->setUniformRowHeights(true);
  layout->addWidget(treeView);

  unsigned dw = treeView->fontMetrics().width('0');
  treeView->setColumnWidth(0, dw * 4);
  treeView->setColumnWidth(1, dw * 8);
  treeView->setColumnWidth(2, dw * 6);
  treeView->setColumnWidth(3, dw * 6);
  treeView->setColumnWidth(4, dw * 6);
  treeView->setColumnWidth(5, dw * 6);
  treeView->setColumnWidth(6, dw * 6);
  treeView->setColumnWidth(7, dw * 6);

  treeView->setSortingEnabled(true);
  treeView->sortByColumn(OamDataModel::Columns::ID, Qt::AscendingOrder);


  controlLayout = new QVBoxLayout;
  controlLayout->setAlignment(Qt::AlignTop);
  controlLayout->setSpacing(0);
  layout->addLayout(controlLayout);


  canvas = new OamCanvas(dataModel, this);
  controlLayout->addWidget(canvas);

  autoUpdateBox = new QCheckBox("Auto update");
  controlLayout->addWidget(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  controlLayout->addWidget(refreshButton);


  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(treeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(onSelectionChanged()));
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

  dataModel->refresh();
  canvas->refresh();

  inRefreshCall = false;
}

void OamViewer::onSelectionChanged() {
  QModelIndex index = proxyModel->mapToSource(treeView->currentIndex());
  int s = dataModel->objectId(index);

  canvas->setSelected(s);
  refresh();
}

OamCanvas::OamCanvas(OamDataModel* dataModel, QWidget *parent)
  : QFrame(parent)
  , dataModel(dataModel)
{
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
    refreshImage(dataModel->oamObject(selected));
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

  const QSize objSize = dataModel->sizeOfObject(obj);

  QImage buffer(objSize, QImage::Format_RGB32);
  const uint8_t *objTileset = SNES::memory::vram.data() + SNES::ppu.oam_tile_addr(obj.table);

  for(unsigned ty = 0; ty < objSize.height() / 8; ty++) {
    for(unsigned tx = 0; tx < objSize.width() / 8; tx++) {
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
