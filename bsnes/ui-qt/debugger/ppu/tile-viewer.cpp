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

  source = new QComboBox;
  source->addItem("VRAM", QVariant(TileRenderer::VRAM));
  source->addItem("S-CPU Bus", QVariant(TileRenderer::CPU_BUS));
  source->addItem("Cartridge ROM", QVariant(TileRenderer::CART_ROM));
  source->addItem("Cartridge RAM", QVariant(TileRenderer::CART_RAM));
  source->addItem("SA1 Bus", QVariant(TileRenderer::SA1_BUS));
  source->addItem("SFX Bus", QVariant(TileRenderer::SFX_BUS));
  sidebarLayout->addRow("Source:", source);


  addressLayout = new QHBoxLayout;

  prevAddressButton = new QToolButton;
  prevAddressButton->setToolTip("Previous");
  prevAddressButton->setIcon(QIcon(":16x16/mem-prev-unknown.png"));
  addressLayout->addWidget(prevAddressButton);

  address = new QLineEdit;
  address->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  address->setMinimumWidth(7 * address->fontMetrics().width('0'));
  addressLayout->addWidget(address, 1);

  nextAddressButton = new QToolButton;
  nextAddressButton->setToolTip("Next");
  nextAddressButton->setIcon(QIcon(":16x16/mem-next-unknown.png"));
  addressLayout->addWidget(nextAddressButton);

  sidebarLayout->addRow("Address:", addressLayout);


  bitDepth = new QComboBox;
  bitDepth->addItem("2bpp", QVariant(TileRenderer::BPP2));
  bitDepth->addItem("4bpp", QVariant(TileRenderer::BPP4));
  bitDepth->addItem("8bpp", QVariant(TileRenderer::BPP8));
  bitDepth->addItem("Mode 7", QVariant(TileRenderer::MODE7));
  bitDepth->addItem("Mode 7 EXTBG", QVariant(TileRenderer::MODE7_EXTBG));
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

  connect(source,        SIGNAL(activated(int)),             this, SLOT(refresh()));
  connect(address,       SIGNAL(textEdited(const QString&)), this, SLOT(refresh()));
  connect(bitDepth,      SIGNAL(activated(int)),             this, SLOT(refresh()));
  connect(widthSpinBox,  SIGNAL(valueChanged(int)),          this, SLOT(refresh()));

  connect(prevAddressButton, SIGNAL(clicked(bool)), this, SLOT(onPrevAddressButtonClicked()));
  connect(nextAddressButton, SIGNAL(clicked(bool)), this, SLOT(onNextAddressButtonClicked()));

  connect(overrideBackgroundColor, SIGNAL(clicked(bool)),    this, SLOT(refresh()));
  connect(customBgColorCombo,      SIGNAL(activated(int)),   this, SLOT(refresh()));

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
  if(inUpdateFormCall || inExportClickedCall) return;

  updateRendererSettings();

  if(SNES::cartridge.loaded()) {
    cgramWidget->refresh();

    renderer.draw();
    imageGridWidget->setImage(renderer.image);
  }

  updateForm();
  updateTileInfo();
}

void TileViewer::onZoomChanged(int index) {
  unsigned z = zoomCombo->itemData(index).toUInt();
  imageGridWidget->setZoom(z);
}

void TileViewer::onExportClicked() {
  if(renderer.image.isNull()) return;

  inExportClickedCall = true;

  QString selectedFile = QFileDialog::getSaveFileName(
    this, "Export Tiles", config().path.current.exportVRAM, "PNG Image (*.png)");
    
  if(!selectedFile.isEmpty()) {
    QImageWriter writer(selectedFile, "PNG");
    if(!writer.write(renderer.image)) {
      QMessageBox::critical(this, "Export Tiles", "Unable to export tiles:\n\n" + writer.errorString());
    }
    config().path.current.exportVRAM = selectedFile;
  }

  inExportClickedCall = false;
}

void TileViewer::onUseCgramPressed() {
  if(useCgram->isChecked()) {
    if(!cgramWidget->hasSelected()) cgramWidget->setSelected(0);
  } else {
    cgramWidget->selectNone();
  }
}

void TileViewer::onVramBaseButtonClicked(int index) {
  if(!SNES::cartridge.loaded()) return;

  unsigned addr = getVramBaseAddress(index);

  source->setCurrentIndex(source->findData(TileRenderer::VRAM));
  address->clear();

  TileRenderer::BitDepth bd = TileRenderer::BitDepth::NONE;
  if(index < 4) {
    unsigned screenMode = SNES::ppu.bg_mode() & 7;
    if(screenMode < 7) bd = TileRenderer::bitDepthForLayer(screenMode, index);
  }
  if(index >= 4) bd = TileRenderer::BitDepth::BPP4;

  if(bd != TileRenderer::BitDepth::NONE) {
    bitDepth->setCurrentIndex(bitDepth->findData(bd));
  }

  refresh();
  unsigned tileId = addr / renderer.bytesInbetweenTiles();

  QPoint cell(tileId % renderer.width, tileId / renderer.width);
  imageGridWidget->setSelected(cell);
  imageGridWidget->scrollToCell(cell);
}

void TileViewer::onPrevAddressButtonClicked() {
  stepAdddressField(false);
}

void TileViewer::onNextAddressButtonClicked() {
  stepAdddressField(true);
}

void TileViewer::stepAdddressField(bool forward) {
  unsigned step = renderer.bytesInbetweenTiles();
  if(renderer.source != TileRenderer::VRAM) step *= renderer.nTiles();

  if(forward) {
    unsigned max = renderer.maxAddress();
    unsigned a = renderer.address + step;
    if(a >= max) a = max - step;
    if(step > max) a = 0;
    renderer.address = a;
  } else {
    if(renderer.address >= step) {
      renderer.address -= step;
    } else {
      renderer.address = 0;
    }
  }
  renderer.address &= renderer.addressMask();

  if(renderer.source == TileRenderer::VRAM) {
    address->setText(hex<4>(renderer.address));
  } else {
    address->setText(hex<6>(renderer.address));
  }

  refresh();
}

void TileViewer::updateRendererSettings() {
  typedef TileRenderer::Source Source;
  typedef TileRenderer::BitDepth Depth;

  int si = source->currentIndex();
  renderer.source = si >= 0 ? Source(source->itemData(si).toInt()) : Source::VRAM;

  renderer.address = hex(address->text().toUtf8().data());

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

  source->setCurrentIndex(source->findData(renderer.source));

  bitDepth->setCurrentIndex(bitDepth->findData(renderer.bitDepth));

  cgramWidget->setPaletteSize(renderer.colorsPerTile());

  customBgColorCombo->setEnabled(overrideBackgroundColor->isChecked());

  for(unsigned i = 0; i < N_VRAM_BASE_ITEMS; i++) {
    unsigned a = getVramBaseAddress(i);
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
    unsigned tileAddr = renderer.address + tileId * renderer.bytesInbetweenTiles();
    if(renderer.isMode7()) tileAddr++;

    if(renderer.source == TileRenderer::VRAM) {
      text = string("Selected Tile Address: 0x", hex<4>(tileAddr & 0xffff));
    } else {
      text = string("Selected Tile Address: 0x", hex<6>(tileAddr & 0xffffff));
    }
  } else {
    imageGridWidget->selectNone();
  }

  tileInfo->setText(text);
}

unsigned TileViewer::getVramBaseAddress(unsigned index) {
  if(index < 4) return SNES::ppu.bg_tile_addr(index);
  if(index >= 4 && index < 6) return SNES::ppu.oam_tile_addr(index - 4);
  return 0;
}
