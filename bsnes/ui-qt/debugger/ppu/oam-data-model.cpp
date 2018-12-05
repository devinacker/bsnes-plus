#include "oam-data-model.moc"

const QStringList OamDataModel::COLUMN_STRINGS = {
  QString::fromLatin1("#"),
  QString::fromLatin1("Size"),
  QString::fromLatin1("X"),
  QString::fromLatin1("Y"),
  QString::fromLatin1("Char"),
  QString::fromLatin1("Pri"),
  QString::fromLatin1("Pal"),
  QString::fromLatin1("Flip"),
};

const QStringList OamDataModel::FLIP_STRINGS = {
  QString::fromLatin1(""),
  QString::fromLatin1("H"),
  QString::fromLatin1("V"),
  QString::fromLatin1("HV"),
};

const QVector<OamDataModel::ObjectSizes> OamDataModel::OBJECT_SIZE_TABLE = {
  { QSize(8, 8),    QSize(16, 16),  QString::fromLatin1("8x8"),   QString::fromLatin1("16x16"), 16 },
  { QSize(8, 8),    QSize(32, 32),  QString::fromLatin1("8x8"),   QString::fromLatin1("32x32"), 32 },
  { QSize(8, 8),    QSize(64, 64),  QString::fromLatin1("8x8"),   QString::fromLatin1("64x64"), 64 },
  { QSize(16, 16),  QSize(32, 32),  QString::fromLatin1("16x16"), QString::fromLatin1("32x32"), 32 },
  { QSize(16, 16),  QSize(64, 64),  QString::fromLatin1("16x16"), QString::fromLatin1("64x64"), 64 },
  { QSize(32, 32),  QSize(64, 64),  QString::fromLatin1("32x32"), QString::fromLatin1("64x64"), 64 },
  { QSize(16, 32),  QSize(32, 64),  QString::fromLatin1("16x32"), QString::fromLatin1("32x64"), 64 },
  { QSize(16, 32),  QSize(32, 32),  QString::fromLatin1("16x32"), QString::fromLatin1("32x32"), 32 },
};

OamDataModel::OamDataModel(QObject* parent)
  : QAbstractItemModel(parent)
  , mSizeBase(0)
  , mOamObjects()
{
  assert(COLUMN_STRINGS.size() == N_COLUMNS);
  assert(FLIP_STRINGS.size() == 4);
  assert(OBJECT_SIZE_TABLE == N_OAM_BASE_SIZES);

  for(OamObject& obj : mOamObjects) {
    obj.visible = true;
  }
}

void OamDataModel::refresh() {
  mSizeBase = SNES::ppu.oam_base_size();
  mFirstSprite = SNES::ppu.oam_first_sprite();

  for(unsigned i=0; i < N_OBJECTS; i++) {
    uint8_t d0 = SNES::memory::oam[(i << 2) + 0];
    uint8_t d1 = SNES::memory::oam[(i << 2) + 1];
    uint8_t d2 = SNES::memory::oam[(i << 2) + 2];
    uint8_t d3 = SNES::memory::oam[(i << 2) + 3];
    uint8_t d4 = SNES::memory::oam[512 + (i >> 2)];
    bool x8   = d4 & (1 << ((i & 3) << 1));
    bool size = d4 & (2 << ((i & 3) << 1));

    OamObject& obj = mOamObjects[i];

    obj.xpos = sclip<9>((x8 << 8) + d0);
    obj.ypos = d1;
    obj.character = d2;
    obj.priority = (d3 >> 4) & 3;
    obj.palette = (d3 >> 1) & 7;
    obj.size = size;
    obj.vFlip = d3 & 0x80;
    obj.hFlip = d3 & 0x40;
    obj.table = d3 & 0x01;
  }

  emit dataChanged(createIndex(0, 0), createIndex(N_OBJECTS-1, N_COLUMNS-1));
}

QSize OamDataModel::sizeOfObject(const OamObject& obj) const
{
  const ObjectSizes& sizes =  objectSizes();
  return obj.size == false ? sizes.small : sizes.large;
}

const QString& OamDataModel::sizeStringOfObject(const OamObject& obj) const
{
  const ObjectSizes& sizes =  objectSizes();
  return obj.size == false ? sizes.smallString : sizes.largeString;
}

const OamObject& OamDataModel::oamObject(const QModelIndex& index) const {
  int id = 0;
  if(index.model() == this && index.isValid()) id = index.row();

  return mOamObjects[id % N_OBJECTS];
}

bool OamDataModel::isIndexValid(const QModelIndex& index) const {
  return index.model() == this
    && index.isValid()
    && index.row() >= 0 && index.row() < N_OBJECTS
    && index.column() >= 0 && index.column() < N_COLUMNS;
}

int OamDataModel::objectId(const QModelIndex& index) const {
  if(isIndexValid(index) == false) return -1;
  return index.row();
}

QModelIndex OamDataModel::objectIdToIndex(int id) const {
  if(id < 0 || id >= N_OBJECTS) return QModelIndex();
  return createIndex(id, 0);
}

bool OamDataModel::hasChildren(const QModelIndex& parent) const {
  return parent.isValid() == false;
}

int OamDataModel::rowCount(const QModelIndex& parent) const {
  if(parent.isValid()) return 0;
  return N_OBJECTS;
}

int OamDataModel::columnCount(const QModelIndex& parent) const {
  if(parent.isValid()) return 0;
  return N_COLUMNS;
}

QModelIndex OamDataModel::index(int row, int column, const QModelIndex& parent) const {
  if(parent.isValid()
     || row < 0 || row >= N_OBJECTS
     || column < 0 || column >= N_COLUMNS) {

    return QModelIndex();
  }

  return createIndex(row, column);
}

QModelIndex OamDataModel::parent(const QModelIndex & index) const {
  return QModelIndex();
}

Qt::ItemFlags OamDataModel::flags(const QModelIndex & index) const {
  if(isIndexValid(index) == false) return 0;

  if(index.column() == ID) {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
  }
  else {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
}

QVariant OamDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if(orientation != Qt::Horizontal
    || section < 0 || section >= N_COLUMNS
    || role != Qt::DisplayRole) {

    return QVariant();
  }

  return COLUMN_STRINGS.at(section);
}

QVariant OamDataModel::data(const QModelIndex& index, int role) const {
  if(isIndexValid(index) == false) return QVariant();

  if(role == Qt::DisplayRole) {
    const OamObject& obj = oamObject(index.row());

    switch(static_cast<Columns>(index.column())) {
      case ID:        return index.row();
      case SIZE:      return sizeStringOfObject(obj);
      case XPOS:      return obj.xpos;
      case YPOS:      return obj.ypos;
      case CHAR:      return obj.character + (obj.table << 8);
      case PRIORITY:  return obj.priority;
      case PALETTE:   return obj.palette;
      case FLIP:      return FLIP_STRINGS.at(obj.hFlip | (obj.vFlip << 1));
    }
  }
  else if(role == Qt::CheckStateRole) {
    if(index.column() == ID) {
      const OamObject& obj = oamObject(index.row());
      return obj.visible ? Qt::Checked : Qt::Unchecked;
    }
  }
  else if(role == SortRole) {
    const OamObject& obj = oamObject(index.row());
    switch(static_cast<Columns>(index.column())) {
      case ID:        return index.row();
      case SIZE:      return obj.size;
      case XPOS:      return obj.xpos;
      case YPOS:      return obj.ypos;
      case CHAR:      return obj.character + (obj.table << 8);
      case PRIORITY:  return obj.priority;
      case PALETTE:   return obj.palette;
      case FLIP:      return obj.hFlip | (obj.vFlip << 1);
    }
  }
  else if(role == Qt::TextAlignmentRole) {
    switch(static_cast<Columns>(index.column())) {
      case ID:        return Qt::AlignRight;
      case SIZE:      return Qt::AlignHCenter;
      case XPOS:      return Qt::AlignRight;
      case YPOS:      return Qt::AlignRight;
      case CHAR:      return Qt::AlignRight;
      case PRIORITY:  return Qt::AlignRight;
      case PALETTE:   return Qt::AlignRight;
      case FLIP:      return Qt::AlignLeft;
    }
  }

  return QVariant();
}

bool OamDataModel::setData(const QModelIndex& index, const QVariant& value, int role) {
  if(isIndexValid(index) == false) return false;

  if(role == Qt::CheckStateRole) {
    if(index.column() == ID) {
      unsigned id = index.row();
      mOamObjects[id % N_OBJECTS].visible = (value == Qt::Checked);

      emit dataChanged(index, index);
      emit visibilityChanged();

      return true;
    }
  }

  return false;
}

void OamDataModel::showAllObjects() {
  for(OamObject& obj : mOamObjects) {
    obj.visible = true;
  }

  emit dataChanged(createIndex(0, ID), createIndex(N_OBJECTS-1, ID));
  emit visibilityChanged();
}

void OamDataModel::toggleVisibility(const QSet<int>& objectIds) {
  for(int id : objectIds) {
    if(id >= 0 && id < N_OBJECTS) {
      mOamObjects[id].visible = !mOamObjects[id].visible;
    }
  }

  emit dataChanged(createIndex(0, ID), createIndex(N_OBJECTS-1, ID));
  emit visibilityChanged();
}

void OamDataModel::showOnlySelectedObjects(const QSet<int>& objectIds) {
  for(int id = 0; id < N_OBJECTS; id++) {
    mOamObjects[id].visible = objectIds.contains(id);
  }

  emit dataChanged(createIndex(0, ID), createIndex(N_OBJECTS-1, ID));
  emit visibilityChanged();
}
