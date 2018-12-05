
struct OamObject {
  signed   xpos;
  unsigned ypos;
  unsigned character;
  unsigned priority;
  unsigned palette;
  bool size;
  bool hFlip;
  bool vFlip;
  bool table;

  bool visible;
};

class OamDataModel : public QAbstractItemModel {
  Q_OBJECT

public:
  static const int N_OBJECTS = 128;

  enum Columns {
    ID,
    SIZE,
    XPOS,
    YPOS,
    CHAR,
    PRIORITY,
    PALETTE,
    FLIP,
  };
  static const int N_COLUMNS = 8;
  static const QStringList COLUMN_STRINGS;
  static const QStringList FLIP_STRINGS;

  struct ObjectSizes {
    const QSize small;
    const QSize large;
    const QString smallString;
    const QString largeString;
    const unsigned maximumSize;
  };
  static const int N_OAM_BASE_SIZES = 8;
  static const QVector<ObjectSizes> OBJECT_SIZE_TABLE;

  static const int SortRole = Qt::UserRole + 1;

private:
  unsigned mSizeBase;
  unsigned mFirstSprite;
  OamObject mOamObjects[N_OBJECTS];

public:
  OamDataModel(QObject* parent);

  void refresh();

  const ObjectSizes& objectSizes() const { return OBJECT_SIZE_TABLE.at(mSizeBase % N_OAM_BASE_SIZES); }
  QSize sizeOfObject(const OamObject& obj) const;
  const QString& sizeStringOfObject(const OamObject& obj) const;

  unsigned firstSprite() const { return mFirstSprite; }

  const OamObject& oamObject(unsigned id) const { return mOamObjects[id % N_OBJECTS]; }
  const OamObject& oamObject(const QModelIndex& index) const;

  bool isIndexValid(const QModelIndex& index) const;

  // returns -1 if the index is invalid
  int objectId(const QModelIndex& index) const;

  QModelIndex objectIdToIndex(int id) const;

  virtual bool hasChildren(const QModelIndex& parent) const override;
  virtual int rowCount(const QModelIndex& parent) const override;
  virtual int columnCount(const QModelIndex& parent) const override;

  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
  virtual QModelIndex parent(const QModelIndex & index) const override;

  virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  virtual QVariant data(const QModelIndex& index, int role) const override;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  void toggleVisibility(const QSet<int>& objectIds);
  void showOnlySelectedObjects(const QSet<int>& objectIds);

public slots:
  void showAllObjects();

signals:
  void visibilityChanged();
};

