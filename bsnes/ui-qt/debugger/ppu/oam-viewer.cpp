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

  graphicsScene = new OamGraphicsScene(dataModel, this);

  outerLayout = new QVBoxLayout;
  setLayout(outerLayout);

  splitter = new QSplitter;
  splitter->setChildrenCollapsible(false);
  splitter->setOrientation(Qt::Vertical);
  outerLayout->addWidget(splitter);

  graphicsView = new QGraphicsView;
  graphicsView->setMinimumSize(256, 256);
  graphicsView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  graphicsView->setScene(graphicsScene);
  splitter->addWidget(graphicsView);

  bottomWidget = new QWidget;
  splitter->addWidget(bottomWidget);

  layout = new QHBoxLayout;
  layout->setAlignment(Qt::AlignLeft);
  layout->setMargin(Style::WindowMargin);
  layout->setSpacing(Style::WidgetSpacing);
  bottomWidget->setLayout(layout);

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


  sidebarLayout = new QFormLayout;
  sidebarLayout->setSizeConstraint(QLayout::SetMinimumSize);
  sidebarLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
  sidebarLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  sidebarLayout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
  sidebarLayout->setLabelAlignment(Qt::AlignLeft);
  layout->addLayout(sidebarLayout);

  canvas = new OamCanvas(dataModel, graphicsScene, this);
  sidebarLayout->addRow(canvas);

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
  sidebarLayout->addRow("Zoom:", zoomCombo);

  autoUpdateBox = new QCheckBox("Auto update");
  sidebarLayout->addRow(autoUpdateBox);

  refreshButton = new QPushButton("Refresh");
  sidebarLayout->addRow(refreshButton);


  splitter->setSizes({ INT_MAX / 100 * 2, INT_MAX / 100 * 1 });
  splitter->setStretchFactor(0, 2);
  splitter->setStretchFactor(1, 1);


  zoomCombo->setCurrentIndex(0);
  onZoomChanged(0);


  connect(refreshButton, SIGNAL(released()), this, SLOT(refresh()));
  connect(zoomCombo,     SIGNAL(currentIndexChanged(int)), this, SLOT(onZoomChanged(int)));
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
  graphicsScene->refresh();
  canvas->refresh();

  inRefreshCall = false;
}

void OamViewer::onZoomChanged(int index)
{
  unsigned z = zoomCombo->itemData(index).toUInt();
  graphicsView->setTransform(QTransform::fromScale(z, z));
}

void OamViewer::onSelectionChanged() {
  QModelIndex index = proxyModel->mapToSource(treeView->currentIndex());
  int s = dataModel->objectId(index);

  canvas->setSelected(s);
}

OamCanvas::OamCanvas(OamDataModel* dataModel, OamGraphicsScene* graphicsScene, QWidget *parent)
  : QFrame(parent)
  , dataModel(dataModel)
  , graphicsScene(graphicsScene)
  , selected(-1)
  , backgroundColor()
  , pixmap()
  , pixmapScale(1)
{
  setFrameStyle(QFrame::Shape::Panel | QFrame::Sunken);
  setLineWidth(2);

  selected = -1;
  setScale(2);
}

void OamCanvas::paintEvent(QPaintEvent*e) {
  QFrame::paintEvent(e);

  if(!pixmap.isNull()) {
    QPainter painter(this);
    painter.setRenderHints(0);

    unsigned x = (width() - pixmapScale * pixmap.width()) / 2;
    unsigned y = (height() - pixmapScale * pixmap.height()) / 2;

    painter.translate(x, y);
    painter.scale(pixmapScale, pixmapScale);

    painter.fillRect(0, 0, pixmap.width(), pixmap.height(), backgroundColor);
    painter.drawPixmap(0, 0, pixmap);
  }
}

void OamCanvas::setScale(unsigned z) {
  imageSize = 64 * z;
  setFixedSize(imageSize + frameWidth() * 2, imageSize + frameWidth() * 2);
  refresh();
}

void OamCanvas::setSelected(int s) {
  selected = s;
  refresh();
}

void OamCanvas::refresh() {
  backgroundColor = graphicsScene->backgroundColorForObject(selected);
  pixmap = graphicsScene->pixmapForObject(selected);
  pixmapScale = imageSize / dataModel->objectSizes().maximumSize;

  update();
}

