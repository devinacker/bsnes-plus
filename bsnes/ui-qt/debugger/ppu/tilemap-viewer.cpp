#include "tilemap-viewer.moc"

TilemapViewer *tilemapViewer;

TilemapViewer::TilemapViewer() {
  setObjectName("tilemap-viewer");
  setWindowTitle("Tilemap Viewer");
  setGeometryString(&config().geometry.tilemapViewer);
  application.windowList.append(this);

  inUpdateFormCall = false;
  inExportClickedCall = false;

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

  showGrid = new QCheckBox("Show Grid");
  sidebarLayout->addRow(zoomCombo, showGrid);

  autoUpdateBox = new QCheckBox("Auto update");
  sidebarLayout->addRow("", autoUpdateBox);


  buttonLayout = new QHBoxLayout;

  exportButton = new QPushButton("Export");
  buttonLayout->addWidget(exportButton);

  refreshButton = new QPushButton("Refresh");
  buttonLayout->addWidget(refreshButton);

  sidebarLayout->addRow(buttonLayout);


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
  bitDepth->addItem("Mode 7 EXTBG", QVariant(TileRenderer::MODE7_EXTBG));
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

  sidebarLayout->addRow(new QWidget);


  overrideBackgroundColor = new QCheckBox("Override Background Color");
  sidebarLayout->addRow(overrideBackgroundColor);

  customBgColorCombo = new QComboBox;
  customBgColorCombo->addItem("Transparent", QVariant(qRgba(0, 0, 0, 0)));
  customBgColorCombo->addItem("Magenta",     QVariant(qRgb(255, 0, 255)));
  customBgColorCombo->addItem("Cyan",        QVariant(qRgb(0, 255, 255)));
  customBgColorCombo->addItem("White",       QVariant(qRgb(255, 255, 255)));
  customBgColorCombo->addItem("Black",       QVariant(qRgb(0, 0, 0)));
  sidebarLayout->addRow("BG Color:", customBgColorCombo);


  sidebarLayout->addRow(new QWidget);

  tileInfo = new QLabel;
  sidebarLayout->addRow(tileInfo);


  imageGridWidget = new ImageGridWidget();
  imageGridWidget->setMinimumSize(256, 256);
  layout->addWidget(imageGridWidget, 10);


  updateForm();


  connect(exportButton,  SIGNAL(clicked(bool)), this, SLOT(onExportClicked()));
  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoomCombo,     SIGNAL(currentIndexChanged(int)), this, SLOT(onZoomChanged(int)));
  connect(showGrid,      SIGNAL(clicked(bool)), imageGridWidget, SLOT(setShowGrid(bool)));

  connect(customScreenMode, SIGNAL(clicked(bool)), this, SLOT(refresh()));
  connect(customTilemap,    SIGNAL(clicked(bool)), this, SLOT(refresh()));

  for(int i = 0; i < 4; i++) {
    connect(bgButtons[i],SIGNAL(clicked(bool)),               this, SLOT(refresh()));
  }
  connect(screenMode,    SIGNAL(valueChanged(int)),          this, SLOT(refresh()));
  connect(bitDepth,      SIGNAL(activated(int)),             this, SLOT(refresh()));
  connect(screenSize,    SIGNAL(activated(int)),             this, SLOT(refresh()));
  connect(tileSize,      SIGNAL(activated(int)),             this, SLOT(refresh()));
  connect(tileAddr,      SIGNAL(textEdited(const QString&)), this, SLOT(refresh()));
  connect(screenAddr,    SIGNAL(textEdited(const QString&)), this, SLOT(refresh()));

  connect(overrideBackgroundColor, SIGNAL(clicked(bool)),    this, SLOT(refresh()));
  connect(customBgColorCombo,      SIGNAL(activated(int)),   this, SLOT(refresh()));

  connect(imageGridWidget, SIGNAL(selectedChanged()),         this, SLOT(refresh()));
}

void TilemapViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void TilemapViewer::show() {
  Window::show();
  refresh();
}

void TilemapViewer::refresh() {
  if(inUpdateFormCall || inExportClickedCall) return;

  updateRendererSettings();

  if(SNES::cartridge.loaded()) {
    renderer.drawTilemap();
    imageGridWidget->setImage(renderer.image);
    imageGridWidget->setGridSize(renderer.tileSizePx());
  }

  updateForm();
  updateTileInfo();
}

void TilemapViewer::onZoomChanged(int index) {
  unsigned z = zoomCombo->itemData(index).toUInt();
  imageGridWidget->setZoom(z);
}

void TilemapViewer::onExportClicked() {
  if(renderer.image.isNull()) return;

  inExportClickedCall = true;
  
  QString selectedFile = QFileDialog::getSaveFileName(
    this, "Export Tilemap", config().path.current.exportVRAM, "PNG Image (*.png)");
    
  if(!selectedFile.isEmpty()) {
    QImageWriter writer(selectedFile, "PNG");
    if(!writer.write(renderer.image)) {
      QMessageBox::critical(this, "Export Tilemap", "Unable to export tilemap:\n\n" + writer.errorString());
    }
    config().path.current.exportVRAM = selectedFile;
  }

  inExportClickedCall = false;
}

void TilemapViewer::updateRendererSettings() {
  unsigned layer = 0;
  for(int i = 0; i < 4; i++) {
    if(bgButtons[i]->isChecked()) layer = i;
  }

  renderer.layer = layer;

  if(customScreenMode->isChecked()) {
    renderer.screenMode = screenMode->value();
  } else {
    renderer.loadScreenMode();
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
  } else {
    renderer.loadTilemapSettings();
  }

  int i = customBgColorCombo->currentIndex();
  renderer.overrideBackgroundColor = overrideBackgroundColor->isChecked();
  renderer.customBackgroundColor = customBgColorCombo->itemData(i).toUInt();
}

void TilemapViewer::updateForm() {
  inUpdateFormCall = true;

  exportButton->setEnabled(!renderer.image.isNull());

  bool csm = customScreenMode->isChecked();
  screenMode->setEnabled(csm);

  if(csm == false) {
    screenMode->setValue(renderer.screenMode);
  }

  unsigned nLayers = renderer.nLayersInMode();
  if(!csm && renderer.screenMode == 7) nLayers = 0;
  for(unsigned i = 0; i < 4; i++) {
    bgButtons[i]->setChecked(i == renderer.layer);
    bgButtons[i]->setEnabled(i < nLayers);
  }

  bool ct = customTilemap->isChecked();
  bool mode7 = renderer.isMode7();

  bitDepth->setEnabled(ct);
  screenAddr->setEnabled(ct & !mode7);
  tileAddr->setEnabled(ct & !mode7);
  screenSize->setEnabled(ct & !mode7);
  tileSize->setEnabled(ct & !mode7);

  if(ct == false) {
    renderer.updateBitDepth();
    bitDepth->setCurrentIndex(bitDepth->findData(renderer.bitDepth));

    unsigned ss = (renderer.screenSizeY << 1) | int(renderer.screenSizeX);

    screenAddr->setText(string("0x", hex<4>(renderer.screenAddr)));
    tileAddr->setText(string("0x", hex<4>(renderer.tileAddr)));
    screenSize->setCurrentIndex(ss);
    tileSize->setCurrentIndex(renderer.tileSize);
  }

  customBgColorCombo->setEnabled(overrideBackgroundColor->isChecked());

  inUpdateFormCall = false;
}

void TilemapViewer::updateTileInfo() {
  if(SNES::cartridge.loaded() && imageGridWidget->selectionValid()) {
    if(!renderer.isMode7()) {
      updateTileInfoNormal();
    } else {
      updateTileInfoMode7();
    }
  }
  else {
    tileInfo->clear();
  }
}

void TilemapViewer::updateTileInfoNormal() {
  const uint8_t *vram = SNES::memory::vram.data();

  unsigned xPos = imageGridWidget->selected().x() & 0x3f;
  unsigned yPos = imageGridWidget->selected().y() & 0x3f;

  unsigned mapId = 0;
  if(xPos >= 32 && renderer.screenSizeX) mapId += 1;
  if(yPos >= 32 && renderer.screenSizeY) mapId += (renderer.screenSizeX) ? 2 : 1;

  xPos &= 0x1f;
  yPos &= 0x1f;

  unsigned tileAddr = (renderer.screenAddr + mapId * 0x800 + yPos * 64 + xPos * 2) & 0xfffe;
  unsigned tile = vram[tileAddr] | (vram[tileAddr + 1] << 8);

  unsigned character = tile & 0x03ff;
  unsigned pal = (tile >> 10) & 7;
  bool priority = tile & 0x2000;
  bool hFlip = tile & 0x4000;
  bool vFlip = tile & 0x8000;

  unsigned charAddr = renderer.characterAddress(character);

  string text;

  text << "<table>";
  text << "<tr><td>Map: </td><td>" << mapId << "</td></tr>";
  text << "<tr><td>Position: </td><td>" << xPos << ", " << yPos << "</td></tr>";
  text << "<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
  text << "<tr><td>Address: </td><td>0x" << hex<4>(tileAddr) << "</td></tr>";
  text << "<tr><td>Value: </td><td>0x" << hex<4>(tile) << "</td></tr>";
  text << "<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
  text << "<tr><td>Character: </td><td>" << character << "</td></tr>";
  text << "<tr><td>Char Address: </td><td>0x" << hex<4>(charAddr) << "</td></tr>";
  text << "<tr><td>Palette: </td><td>" << pal << "</td></tr>";
  text << "<tr><td>Priority: </td><td>" << (unsigned)priority << "</td></tr>";
  text << "<tr><td>hFlip: </td><td>" << (unsigned)hFlip << "</td></tr>";
  text << "<tr><td>vFlip: </td><td>" << (unsigned)vFlip << "</td></tr>";
  text << "</table>";

  tileInfo->setText(text);
}

void TilemapViewer::updateTileInfoMode7() {
  const uint8_t *vram = SNES::memory::vram.data();

  int xPos = imageGridWidget->selected().x() & 0x7f;
  int yPos = imageGridWidget->selected().y() & 0x7f;

  unsigned addr = yPos * 256 + xPos * 2;
  unsigned tile = vram[addr];

  unsigned charAddr = renderer.characterAddress(tile);

  string text;

  text << "<table>";
  text << "<tr><td>Position: </td><td>" << xPos << ", " << yPos << "</td></tr>";
  text << "<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
  text << "<tr><td>Address: </td><td>0x" << hex<4>(addr) << "</td></tr>";
  text << "<tr><td>Value: </td><td>" << tile << "</td></tr>";
  text << "<tr><td>Char Address: </td><td>0x" << hex<4>(charAddr) << "</td></tr>";
  text << "</table>";

  tileInfo->setText(text);
}
