#include "tile-viewer.moc"

TileViewer *tileViewer;

const char* TileViewer::VramBaseText[6] = {
  "BG1:", "BG2:", "BG3:", "BG4:",
  "OAM1:", "OAM2:"
};

TileViewer::TileViewer() {
  setObjectName("tile-viewer");
  setWindowTitle("Tile Viewer");
  setGeometryString(&config().geometry.tileViewer);
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

  bitDepth = new QComboBox;
  bitDepth->addItem("2bpp", QVariant(TileRenderer::BPP2));
  bitDepth->addItem("4bpp", QVariant(TileRenderer::BPP4));
  bitDepth->addItem("8bpp", QVariant(TileRenderer::BPP8));
  bitDepth->addItem("Mode 7", QVariant(TileRenderer::MODE7));
  sidebarLayout->addRow("Bit Depth:", bitDepth);

  widthSpinBox = new QSpinBox;
  widthSpinBox->setMinimum(8);
  widthSpinBox->setMaximum(64);
  widthSpinBox->setValue(16);
  sidebarLayout->addRow("Width:", widthSpinBox);


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

  useCgram = new QCheckBox("Use CGRAM");
  sidebarLayout->addRow(useCgram);

  cgramWidget = new CgramWidget;
  cgramWidget->setScale(12);
  sidebarLayout->addRow(cgramWidget);


  sidebarLayout->addRow(new QWidget);

  sidebarLayout->addRow(new QLabel("Base Tile Addresses:"));

  vramBaseLayout = new QGridLayout;
  vramBaseLayout->setColumnStretch(0, 1);
  sidebarLayout->addRow(vramBaseLayout);

  vramBaseButtonGroup = new QButtonGroup(this);

  for(unsigned i = 0; i < N_VRAM_BASE_ITEMS; i++) {
    QLabel* label = new QLabel(VramBaseText[i]);
    vramBaseLayout->addWidget(label, i, 1);

    vramBaseAddress[i] = new QLineEdit;
    vramBaseAddress[i]->setReadOnly(true);
    vramBaseAddress[i]->setFixedWidth(9 * vramBaseAddress[i]->fontMetrics().width('0'));
    vramBaseLayout->addWidget(vramBaseAddress[i], i, 2);

    vramBaseButton[i] = new QToolButton;
    vramBaseButton[i]->setText("goto");
    vramBaseLayout->addWidget(vramBaseButton[i], i, 3);

    vramBaseButtonGroup->addButton(vramBaseButton[i], i);
  }


  sidebarLayout->addRow(new QWidget);

  tileInfo = new QLabel;
  sidebarLayout->addRow(tileInfo);


  imageGridWidget = new ImageGridWidget();
  imageGridWidget->setMinimumSize(256, 256);
  imageGridWidget->setGridSize(8);
  imageGridWidget->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  layout->addWidget(imageGridWidget, 10);


  zoomCombo->setCurrentIndex(3 - 1);
  onZoomChanged(3 - 1);
  updateForm();

  connect(exportButton,  SIGNAL(clicked(bool)), this, SLOT(onExportClicked()));
  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoomCombo,     SIGNAL(currentIndexChanged(int)), this, SLOT(onZoomChanged(int)));
  connect(showGrid,      SIGNAL(clicked(bool)), imageGridWidget, SLOT(setShowGrid(bool)));

  connect(bitDepth,      SIGNAL(currentIndexChanged(int)),    this, SLOT(refresh()));
  connect(widthSpinBox,  SIGNAL(valueChanged(int)),           this, SLOT(refresh()));

  connect(overrideBackgroundColor, SIGNAL(clicked(bool)), this, SLOT(refresh()));
  connect(customBgColorCombo,      SIGNAL(currentIndexChanged(int)), this, SLOT(refresh()));

  connect(useCgram,    SIGNAL(clicked()), this, SLOT(onUseCgramPressed()));
  connect(cgramWidget, SIGNAL(selectedChanged()), this, SLOT(refresh()));

  connect(imageGridWidget, SIGNAL(selectedChanged()), this, SLOT(updateTileInfo()));

  connect(vramBaseButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onVramBaseButtonClicked(int)));
}

void TileViewer::autoUpdate() {
  if(autoUpdateBox->isChecked()) refresh();
}

void TileViewer::show() {
  Window::show();
  refresh();
}

void TileViewer::refresh() {
  if(inUpdateFormCall) return;

  if(SNES::cartridge.loaded()) {
    cgramWidget->refresh();

    updateRendererSettings();

    renderer.draw();
    imageGridWidget->setImage(renderer.image);

    updateForm();
  }

  updateTileInfo();
}

void TileViewer::onZoomChanged(int index) {
  unsigned z = zoomCombo->itemData(index).toUInt();
  imageGridWidget->setZoom(z);
}

void TileViewer::onExportClicked() {
  if(renderer.image.isNull()) return;

  QFileDialog saveDialog(this, "Export Tiles");
  saveDialog.setAcceptMode(QFileDialog::AcceptSave);
  saveDialog.setNameFilter("PNG Image (*.png)");
  saveDialog.setDefaultSuffix("png");
  saveDialog.exec();

  QString filename = saveDialog.selectedFiles().first();

  if(saveDialog.result() == QDialog::Accepted && !filename.isEmpty()) {
    QImageWriter writer(filename, "PNG");
    bool s = writer.write(renderer.image);
    if(s == false) {
      QMessageBox::critical(this, "ERROR", "Unable to export tiles\n\n" + writer.errorString());
    }
  }
}

void TileViewer::onUseCgramPressed() {
  if(useCgram->isChecked()) {
    if(!cgramWidget->hasSelected()) cgramWidget->setSelected(0);
  } else {
    cgramWidget->selectNone();
  }
}

void TileViewer::onVramBaseButtonClicked(int index) {
  unsigned addr = hex(vramBaseAddress[index]->text().toUtf8().data());

  unsigned tileId = addr / renderer.bytesInbetweenTiles();

  QPoint cell(tileId % renderer.width, tileId / renderer.width);
  imageGridWidget->setSelected(cell);
  imageGridWidget->scrollToCell(cell);
}

void TileViewer::updateRendererSettings() {
  typedef TileRenderer::BitDepth Depth;

  int bd = bitDepth->currentIndex();
  renderer.bitDepth = bd >= 0 ? Depth(bitDepth->itemData(bd).toInt()) : Depth::NONE;

  int ci = customBgColorCombo->currentIndex();
  renderer.overrideBackgroundColor = overrideBackgroundColor->isChecked();
  renderer.customBackgroundColor = customBgColorCombo->itemData(ci).toUInt();

  renderer.width = widthSpinBox->value();

  if(cgramWidget->hasSelected()) {
    renderer.paletteOffset = cgramWidget->selectedColor();
    useCgram->setChecked(true);
  }

  renderer.useCgramPalette = useCgram->isChecked();
}

void TileViewer::updateForm() {
  inUpdateFormCall = true;

  exportButton->setEnabled(!renderer.image.isNull());

  bitDepth->setCurrentIndex(bitDepth->findData(renderer.bitDepth));

  cgramWidget->setPaletteSize(renderer.colorsPerTile());

  customBgColorCombo->setEnabled(overrideBackgroundColor->isChecked());

  for(unsigned i = 0; i < N_VRAM_BASE_ITEMS; i++) {
    unsigned a = 0;
    if(i < 4) a = SNES::ppu.bg_tile_addr(i);
    if(i >= 4 && i < 6) a = SNES::ppu.oam_tile_addr(i - 4);

    vramBaseAddress[i]->setText(string("0x", hex<4>(a)));
  }

  inUpdateFormCall = false;
}

void TileViewer::updateTileInfo() {
  if(!SNES::cartridge.loaded()) { tileInfo->clear(); return; }
  if(!imageGridWidget->selectionValid()) { tileInfo->clear(); return; }

  unsigned tileId = imageGridWidget->selected().y() * renderer.width + imageGridWidget->selected().x();

  string text;
  if(tileId < renderer.nTiles()) {
    unsigned tileAddr = tileId * renderer.bytesInbetweenTiles();

    text = string("Selected Tile Address: ", hex<4>(tileAddr));

  } else {
    imageGridWidget->selectNone();
  }

  tileInfo->setText(text);
}
