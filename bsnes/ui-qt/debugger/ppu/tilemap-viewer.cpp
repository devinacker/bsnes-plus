#include "tilemap-viewer.moc"

TilemapViewer *tilemapViewer;

TilemapViewer::TilemapViewer() {
  setObjectName("tilemap-viewer");
  setWindowTitle("Tilemap Viewer");
  setGeometryString(&config().geometry.tilemapViewer);
  application.windowList.append(this);


  layout = new QHBoxLayout;
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  layout->setAlignment(Qt::AlignLeft);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  sidebarLayout = new QFormLayout;
  sidebarLayout->setSizeConstraint(QLayout::SetFixedSize);
  sidebarLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
  sidebarLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  sidebarLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
  sidebarLayout->setLabelAlignment(Qt::AlignLeft);
  layout->addLayout(sidebarLayout);


  zoomCombo = new QComboBox;
  zoomCombo->addItem("1x", QVariant(1));
  zoomCombo->addItem("2x", QVariant(2));
  zoomCombo->addItem("3x", QVariant(3));
  zoomCombo->addItem("4x", QVariant(4));
  zoomCombo->addItem("5x", QVariant(5));
  zoomCombo->addItem("6x", QVariant(6));
  zoomCombo->addItem("7x", QVariant(7));
  zoomCombo->addItem("8x", QVariant(8));
  zoomCombo->addItem("9x", QVariant(9));

  autoUpdateBox = new QCheckBox("Auto update");
  sidebarLayout->addRow(zoomCombo, autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  sidebarLayout->addRow(refreshButton);

  screenMode = new QLabel;
  sidebarLayout->addRow("Screen Mode:", screenMode);

  bgLayout = new QHBoxLayout;
  for (int i = 0; i < 4; i++) {
    bgButton[i] = new QRadioButton;
    bgButton[i]->setText(QString::number(i + 1));
    connect(bgButton[i], SIGNAL(clicked()), this, SLOT(refresh()));
    bgLayout->addWidget(bgButton[i]);
  }
  bgButton[0]->setChecked(true);
  sidebarLayout->addRow("BG Layer:", bgLayout);

  bitDepth = new QLabel;
  sidebarLayout->addRow("Bit Depth:", bitDepth);

  screenSize = new QLabel;
  sidebarLayout->addRow("Map Size:", screenSize);

  screenAddr = new QLabel;
  sidebarLayout->addRow("Map Addr:", screenAddr);

  tileSize = new QLabel;
  sidebarLayout->addRow("Tile Size:", tileSize);

  tileAddr = new QLabel;
  sidebarLayout->addRow("Tile Addr:", tileAddr);

  layer = 0;

  scene = new QGraphicsScene;

  scenePixmap = new QGraphicsPixmapItem();
  scenePixmap->setTransformationMode(Qt::FastTransformation);
  scene->addItem(scenePixmap);

  view = new QGraphicsView(scene);
  view->setMinimumSize(256, 256);
  layout->addWidget(view, 10);


  updateInfo();


  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoomCombo,     SIGNAL(currentIndexChanged(int)),    this, SLOT(onZoomChanged(int)));
}

void TilemapViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void TilemapViewer::show() {
  Window::show();
  refresh();
}

void TilemapViewer::updateInfo() {
  typedef TilemapRenderer::BitDepth Depth;
  static const Depth bitDepth[8][4] = {
    // mode 0, 2/2/2/2bpp
    {Depth::BPP2, Depth::BPP2, Depth::BPP2, Depth::BPP2},
    // mode 1, 4/4/2bpp
    {Depth::BPP4, Depth::BPP4, Depth::BPP2, Depth::None},
    // mode 2, 4/4bpp + OPT
    {Depth::BPP4, Depth::BPP4, Depth::None, Depth::None},
    // mode 3, 8/4bpp
    {Depth::BPP8, Depth::BPP4, Depth::None, Depth::None},
    // mode 4, 8/2bpp + OPT
    {Depth::BPP8, Depth::BPP2, Depth::None, Depth::None},
    // mode 5, 4/2bpp + hires
    {Depth::BPP4, Depth::BPP2, Depth::None, Depth::None},
    // mode 6, 4bpp + hires + OPT
    {Depth::BPP4, Depth::None, Depth::None, Depth::None},
    // no proper mode 7 support yet
    {Depth::None, Depth::None, Depth::None, Depth::None}
  };
  
  static const char* strDepth[] = { "n/a", "2 BPP", "4 BPP", "8 BPP" };
  static const char* strScreenSize[] = { "32x32", "64x32", "32x64", "64x64" };

  unsigned mode = SNES::ppu.bg_mode() & 7;
  this->screenMode->setText(QString::number(mode));
  for (int i = 0; i < 4; i++) {
    this->bgButton[i]->setEnabled(bitDepth[mode][i] != Depth::None);
    if (this->bgButton[i]->isChecked()) {
      this->layer = i;
    }
  }
  
  unsigned screenSize = SNES::ppu.bg_screen_size(this->layer);
  
  renderer.tileAddr = SNES::ppu.bg_tile_addr(this->layer);
  this->tileAddr->setText(string("0x", hex<4>(renderer.tileAddr)));
  
  renderer.screenAddr = SNES::ppu.bg_screen_addr(this->layer);
  this->screenAddr->setText(string("0x", hex<4>(renderer.screenAddr)));
  
  renderer.bitDepth = bitDepth[mode][this->layer];
  this->bitDepth->setText(strDepth[(int)renderer.bitDepth]);
  
  renderer.screenSizeX = screenSize & 1;
  renderer.screenSizeY = screenSize & 2;
  this->screenSize->setText(strScreenSize[screenSize]);
  
  renderer.tileSize = SNES::ppu.bg_tile_size(this->layer);
  this->tileSize->setText(renderer.tileSize ? "16x16" : "8x8");
}

void TilemapViewer::refresh() {
  if(SNES::cartridge.loaded()) {
    renderer.buildPalette();

    updateInfo();

    if (renderer.bitDepth != TilemapRenderer::BitDepth::None) {
      QImage image = renderer.drawTilemap();
      scenePixmap->setPixmap(QPixmap::fromImage(image));
    } else {
      scenePixmap->setPixmap(QPixmap());
    }

    scene->setSceneRect(scenePixmap->boundingRect());
  }
}

void TilemapViewer::onZoomChanged(int index) {
  unsigned z = zoomCombo->itemData(index).toUInt();
  if(z == 0) z = 1;

  view->resetTransform();
  view->scale(z, z);

  scene->setSceneRect(scenePixmap->boundingRect());
}
