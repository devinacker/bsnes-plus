#include "tilemap-viewer.moc"

TilemapViewer *tilemapViewer;

TilemapViewer::TilemapViewer() {
  setObjectName("tilemap-viewer");
  setWindowTitle("Tilemap Viewer");
  setGeometryString(&config().geometry.tilemapViewer);
  application.windowList.append(this);

  inUpdateFormCall = false;

  layout = new QHBoxLayout;
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  layout->setAlignment(Qt::AlignLeft);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  setLayout(layout);

  sidebarLayout = new QFormLayout;
  sidebarLayout->setSizeConstraint(QLayout::SetMinimumSize);
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

  sidebarLayout->addRow(new QWidget);


  customScreenMode = new QCheckBox("Custom Screen Mode");
  sidebarLayout->addRow(customScreenMode);

  screenMode = new QSpinBox;
  screenMode->setMinimum(0);
  screenMode->setMaximum(7);
  sidebarLayout->addRow("Mode:", screenMode);

  bgLayout = new QHBoxLayout;
  bgLayout->setSizeConstraint(QLayout::SetMinimumSize);
  for(unsigned i = 0; i < 4; i++) {
    bgButtons[i] = new QToolButton;
    bgButtons[i]->setText(QString::number(i + 1));
    bgButtons[i]->setCheckable(true);
    bgButtons[i]->setAutoExclusive(true);
    bgLayout->addWidget(bgButtons[i]);
  }
  sidebarLayout->addRow("BG:", bgLayout);

  sidebarLayout->addRow(new QWidget);


  customTilemap = new QCheckBox("Override Tilemap");
  sidebarLayout->addRow(customTilemap);

  bitDepth = new QComboBox;
  bitDepth->addItem("2bpp", QVariant(TilemapRenderer::BPP2));
  bitDepth->addItem("4bpp", QVariant(TilemapRenderer::BPP4));
  bitDepth->addItem("8bpp", QVariant(TilemapRenderer::BPP8));
  bitDepth->addItem("Mode 7", QVariant(TilemapRenderer::MODE7));
  sidebarLayout->addRow("Bit Depth:", bitDepth);

  screenSize = new QComboBox;
  screenSize->addItem("32x32", QVariant(0));
  screenSize->addItem("64x32", QVariant(1));
  screenSize->addItem("32x64", QVariant(2));
  screenSize->addItem("64x64", QVariant(3));
  sidebarLayout->addRow("Map Size:", screenSize);

  screenAddr = new QLineEdit;
  sidebarLayout->addRow("Map Addr:", screenAddr);

  tileSize = new QComboBox;
  tileSize->addItem("8x8", QVariant(false));
  tileSize->addItem("16x16", QVariant(true));
  sidebarLayout->addRow("Tile Size:", tileSize);

  tileAddr = new QLineEdit;
  sidebarLayout->addRow("Tile Addr:", tileAddr);

  setCustomScreenMode(false);
  setCustomTilemap(false);

  scene = new QGraphicsScene;

  scenePixmap = new QGraphicsPixmapItem();
  scenePixmap->setTransformationMode(Qt::FastTransformation);
  scene->addItem(scenePixmap);

  view = new QGraphicsView(scene);
  view->setMinimumSize(256, 256);
  layout->addWidget(view, 10);


  onFormChanged();


  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoomCombo,     SIGNAL(currentIndexChanged(int)), this, SLOT(onZoomChanged(int)));

  connect(customScreenMode, SIGNAL(clicked(bool)), this, SLOT(setCustomScreenMode(bool)));
  connect(customTilemap,    SIGNAL(clicked(bool)), this, SLOT(setCustomTilemap(bool)));

  for(int i = 0; i < 4; i++) {
    connect(bgButtons[i],SIGNAL(clicked(bool)),               this, SLOT(onFormChanged()));
  }
  connect(screenMode,    SIGNAL(valueChanged(int)),           this, SLOT(onFormChanged()));
  connect(bitDepth,      SIGNAL(currentIndexChanged(int)),    this, SLOT(onFormChanged()));
  connect(screenSize,    SIGNAL(currentIndexChanged(int)),    this, SLOT(onFormChanged()));
  connect(tileSize,      SIGNAL(currentIndexChanged(int)),    this, SLOT(onFormChanged()));
  connect(tileAddr,      SIGNAL(textChanged(const QString&)), this, SLOT(onFormChanged()));
  connect(screenAddr,    SIGNAL(textChanged(const QString&)), this, SLOT(onFormChanged()));
}

void TilemapViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void TilemapViewer::show() {
  Window::show();
  refresh();
}

void TilemapViewer::refresh() {
  if(SNES::cartridge.loaded()) {
    bool loadMode = customScreenMode->isChecked() == false;
    bool loadTilemap = customTilemap->isChecked() == false;

    if(loadMode) renderer.loadScreenMode();
    if(loadTilemap) renderer.loadTilemapSettings();
    if(loadMode || loadTilemap) updateForm();

    renderer.buildPalette();

    QImage image = renderer.drawTilemap();
    scenePixmap->setPixmap(QPixmap::fromImage(image));

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

void TilemapViewer::onFormChanged() {
  if(inUpdateFormCall) return;

  unsigned layer = 0;
  for(int i = 0; i < 4; i++) {
    if(bgButtons[i]->isChecked()) layer = i;
  }

  renderer.layer = layer;

  if(customScreenMode->isChecked()) {
    renderer.screenMode = screenMode->value();
  }

  if(customTilemap->isChecked()) {
    typedef TilemapRenderer::BitDepth Depth;
    int i = bitDepth->currentIndex();
    Depth bd = i >= 0 ? Depth(bitDepth->itemData(i).toInt()) : Depth::NONE;

    renderer.bitDepth = bd;
    renderer.tileAddr = hex(tileAddr->text().toUtf8().data()) & 0xe000;
    renderer.screenAddr = hex(screenAddr->text().toUtf8().data()) & 0xff80;
    renderer.screenSizeX = screenSize->currentIndex() & 1;
    renderer.screenSizeY = screenSize->currentIndex() & 2;
    renderer.tileSize = tileSize->currentIndex();
  }

  refresh();
}

void TilemapViewer::updateForm() {
  inUpdateFormCall = true;

  unsigned nLayers = renderer.nLayersInMode();
  if(renderer.screenMode == 7) nLayers = 0;
  for(unsigned i = 0; i < 4; i++) {
    bgButtons[i]->setChecked(i == renderer.layer);
    bgButtons[i]->setEnabled(i < nLayers);
  }

  if(customScreenMode->isChecked() == false) {
    screenMode->setValue(renderer.screenMode);
  }

  if(customTilemap->isChecked() == false) {
    renderer.updateBitDepth();
    bitDepth->setCurrentIndex(bitDepth->findData(renderer.bitDepth));

    unsigned ss = (renderer.screenSizeY << 1) | int(renderer.screenSizeX);

    screenAddr->setText(string("0x", hex<4>(renderer.screenAddr)));
    tileAddr->setText(string("0x", hex<4>(renderer.tileAddr)));
    screenSize->setCurrentIndex(ss);
    tileSize->setCurrentIndex(renderer.tileSize);
  }

  inUpdateFormCall = false;
}

void TilemapViewer::setCustomScreenMode(bool v) {
  customScreenMode->setChecked(v);

  screenMode->setEnabled(v);

  refresh();
}

void TilemapViewer::setCustomTilemap(bool v) {
  customTilemap->setChecked(v);

  bitDepth->setEnabled(v);
  screenAddr->setEnabled(v);
  tileAddr->setEnabled(v);
  screenSize->setEnabled(v);
  tileSize->setEnabled(v);

  refresh();
}
