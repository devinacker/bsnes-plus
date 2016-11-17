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

  sidebarLayout->addRow(new QWidget);

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


  scene = new QGraphicsScene;

  scenePixmap = new QGraphicsPixmapItem();
  scenePixmap->setTransformationMode(Qt::FastTransformation);
  scene->addItem(scenePixmap);

  view = new QGraphicsView(scene);
  view->setMinimumSize(256, 256);
  layout->addWidget(view, 10);


  onFormChanged();


  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoomCombo,     SIGNAL(currentIndexChanged(int)),    this, SLOT(onZoomChanged(int)));

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
  int bd = bitDepth->itemData(bitDepth->currentIndex()).toInt();
  unsigned ms = screenSize->currentIndex();

  renderer.tileAddr = hex(tileAddr->text().toUtf8().data()) & 0xffff;
  renderer.screenAddr = hex(screenAddr->text().toUtf8().data()) & 0xffff;
  renderer.bitDepth = (TilemapRenderer::BitDepth)bd;
  renderer.screenSizeX = ms & 1;
  renderer.screenSizeY = ms & 2;
  renderer.tileSize = tileSize->currentIndex();

  refresh();
}
